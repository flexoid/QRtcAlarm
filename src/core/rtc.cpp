/* QRtcAlarm
 * Copyright (c) 2010, flexoid
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the
 *       above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or
 *       other materials provided with the distribution.
 *     * The names of its contributors may not be used to
 *       endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "rtc.h"

Rtc* Rtc::_self = 0;

Rtc::Rtc()
{
    rtc_root = QString::fromUtf8("/sys/class/rtc");
    rtc_device = QString::fromUtf8("rtc0");
    wakealarm_sysfile = rtc_root + QString::fromUtf8("/") + rtc_device +
            QString::fromUtf8("/") + QString::fromUtf8("wakealarm");
    since_epoch_sysfile = rtc_root + QString::fromUtf8("/") + rtc_device +
            QString::fromUtf8("/") + QString::fromUtf8("since_epoch");
}

int Rtc::getAlarmTime(QDateTime& dateTime)
{
    QString output;
    QFile wakealarm_file(wakealarm_sysfile);
    wakealarm_file.open(QIODevice::ReadOnly);
    if (!wakealarm_file.exists())
    {
        setError(Rtc::WakealarmFileDoesntExist);
        setErrorString(trUtf8("File %1 doesn't exist.").arg(wakealarm_sysfile));
        wakealarm_file.close();
        return -1;
    }
    if (!wakealarm_file.isReadable())
    {
        setError(Rtc::WakealarmFileDoesntReadable);
        setErrorString(trUtf8("Don't have permission to read %1 file.").arg(wakealarm_sysfile));
        wakealarm_file.close();
        return -1;
    }
    output = QVariant(wakealarm_file.readAll()).toString();
    wakealarm_file.close();
    uint time_t = QVariant(output).toUInt();
    if (time_t <= 0)
    {
        setError(Rtc::AlarmIsNotSet);
        setErrorString(trUtf8("Alarm is not set or it is incorrect."));
        return -1;
    }
    dateTime.setTime_t(time_t);

    if (Rtc::timeSpec == Qt::LocalTime)
        dateTime = dateTime.toLocalTime();

    cleanLastError();
    return 0;
}

int Rtc::setTimeSpec(Qt::TimeSpec spec)
{
    if (spec != Qt::LocalTime && spec != Qt::UTC)
    {
        setError(Rtc::InvalidTimeMode);
        setErrorString(trUtf8("Invalid time spec."));
        return -1;
    }
    timeSpec = spec;
    cleanLastError();
    return 0;
}

Qt::TimeSpec Rtc::getTimeSpec()
{
    cleanLastError();
    return timeSpec;
}

int Rtc::setAlarmTime(QDateTime dateTime)
{
    if (dateTime < QDateTime::currentDateTime() &&  !dateTime.isNull())
    {
        setError(Rtc::TimeInThePast);
        setErrorString(trUtf8("The specified time is in the past."));
        return -1;
    }
    QFile wakealarm_file(wakealarm_sysfile);
    wakealarm_file.open(QIODevice::WriteOnly);
    if (!wakealarm_file.exists())
    {
        setError(Rtc::WakealarmFileDoesntExist);
        setErrorString(trUtf8("File %1 doesn't exist.").arg(wakealarm_sysfile));
        wakealarm_file.close();
        return -1;
    }
    if (!wakealarm_file.isWritable())
    {
        setError(Rtc::WakealarmFileDoesntWritable);
        setErrorString(trUtf8("Don't have permission to write %1 file.").arg(wakealarm_sysfile));
        wakealarm_file.close();
        return -1;
    }
    wakealarm_file.write("0"); // Reset current alarm
    wakealarm_file.close();

    if (dateTime.isNull()) //Alarm reseted
    {
        cleanLastError();
        return 0;
    }

    wakealarm_file.open(QIODevice::WriteOnly);
    int wr = wakealarm_file.write(QVariant(dateTime.toTime_t()).toByteArray());
    if (wr == -1)
    {
        setError(Rtc::IsNotWritten);
        setErrorString(trUtf8("New time isn't written in %1.").arg(wakealarm_sysfile));
        wakealarm_file.close();
        return -1;
    }
    wakealarm_file.close();
    cleanLastError();
    return 0;
}

int Rtc::resetAlarmTime()
{
    setAlarmTime(QVariant(0).toDateTime());
    return 0;
}

int Rtc::getSystemTimeSpec()
{
    QString output;
    QFile since_epoch_file(since_epoch_sysfile);
    since_epoch_file.open(QIODevice::ReadOnly);
    if (!since_epoch_file.exists())
    {
        setError(Rtc::SinceEpochFileDoesntExist);
        setErrorString(trUtf8("File %1 doesn't exist.").arg(since_epoch_sysfile));
        since_epoch_file.close();
        return -1;
    }
    if (!since_epoch_file.isReadable())
    {
        setError(Rtc::SinceEpochFileDoesntReadable);
        setErrorString(trUtf8("Don't have permission to read %1 file.").arg(since_epoch_sysfile));
        since_epoch_file.close();
        return -1;
    }
    output = QVariant(since_epoch_file.readAll()).toString();
    since_epoch_file.close();
    uint time_t = QVariant(output).toUInt();

    cleanLastError();

    if (qAbs(static_cast< int >(time_t - QDateTime::currentDateTime().toTime_t())) <= 60) //infelicity
        return Qt::UTC;
    else
        return Qt::LocalTime;
}

void Rtc::cleanLastError()
{
    err = Rtc::NoError;
    errString = "";
}

void Rtc::setError(rtcError error)
{
    err = error;
}

void Rtc::setErrorString(const QString &str)
{
    errString = str;
}

Rtc::rtcError Rtc::error()
{
    return err;
}

QString Rtc::errorString()
{
    return errString;
}
