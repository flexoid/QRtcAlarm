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


#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->settingsBox->hide();
    ui->logBox->hide();

    updateAlarmTime();
    connect(ui->goToAlarmButton, SIGNAL(clicked()), this, SLOT(goToAlarmDate()));
    connect(ui->goToTodayButton, SIGNAL(clicked()), this, SLOT(goToToday()));

    connect(ui->settingsButton, SIGNAL(toggled(bool)), this, SLOT(showSettings(bool)));
    connect(ui->showLogButton, SIGNAL(toggled(bool)), this, SLOT(showLog(bool)));

    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(updateAlarmTime()));
    connect(ui->calendarWidget, SIGNAL(clicked(QDate)), ui->dateTimeEdit, SLOT(setDate(QDate)));
    connect(ui->dateTimeEdit, SIGNAL(dateChanged(QDate)), ui->calendarWidget, SLOT(setSelectedDate(QDate)));
    connect(ui->setAlarmTimeButton, SIGNAL(clicked()), this, SLOT(setAlarmTime()));

    connect(ui->logListWidget->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            ui->logListWidget, SLOT(scrollToBottom())); //Autoscrolling

    connect(ui->logListWidget->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            ui->logBox, SLOT(show())); //Show log when first log message arrived

    connect(ui->cleanLogButton, SIGNAL(clicked()), ui->logListWidget, SLOT(clear())); //Clear the log
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::updateAlarmTime()
{
    Rtc* rtc = Rtc::instance();
    QDateTime alarmTime;
    if (rtc->getAlarmTime(alarmTime) == 0)
    {
        ui->dateTimeEdit->setDateTime(alarmTime);
        ui->calendarWidget->setSelectedDate(alarmTime.date());
        ui->calendarWidget->markAlarmDate(alarmTime.date());

        ui->stateLabel->setText(tr("%1 Alarm is set %2").arg("<font color=green>").arg("</font>"));
        ui->goToAlarmButton->setEnabled(true);
    }
    else
    {
        ui->stateLabel->setText(tr("%1 Alarm isn't set %2").arg("<font color=red>").arg("</font>"));
        ui->goToAlarmButton->setEnabled(false);
    }
}

void MainWindow::goToAlarmDate()
{
    QDate alarmDate = ui->calendarWidget->getMarkedDate();
    ui->calendarWidget->setCurrentPage(alarmDate.year(), alarmDate.month());
}

void MainWindow::goToToday()
{
    QDate currentDate = QDate::currentDate();
    ui->calendarWidget->setCurrentPage(currentDate.year(), currentDate.month());
}

void MainWindow::showSettings(bool visible)
{
    //TODO: Animation

    ui->settingsBox->setVisible(visible);
}

void MainWindow::showLog(bool visible)
{
    //TODO: Animation

    ui->logBox->setVisible(visible);
}

void MainWindow::setAlarmTime()
{
    Rtc* rtc = Rtc::instance();
    int err = rtc->setAlarmTime(ui->dateTimeEdit->dateTime());
    if (err == 0)
    {
        updateAlarmTime();
    }
    else
    {
        QListWidgetItem* item = new QListWidgetItem(ui->logListWidget);
        item->setText(rtc->errorString());
    }
}
