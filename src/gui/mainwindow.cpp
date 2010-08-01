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
    ui->logBox->hide();

    createActions();
    createMenus();

    connect(ui->goToAlarmButton, SIGNAL(clicked()), this, SLOT(goToAlarmDate()));
    connect(ui->goToCurrentMonthButton, SIGNAL(clicked()), this, SLOT(goToCurrentMonth()));
    connect(ui->goToTodayButton, SIGNAL(clicked()), this, SLOT(goToToday()));

    connect(ui->logButton, SIGNAL(toggled(bool)), this, SLOT(setVisibleLog(bool)));

    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(updateAlarmTime()));
    connect(ui->calendarWidget, SIGNAL(clicked(QDate)), ui->dateTimeEdit, SLOT(setDate(QDate)));
    connect(ui->calendarWidget, SIGNAL(clicked(QDate)), this, SLOT(setFocus()));
    connect(ui->dateTimeEdit, SIGNAL(dateChanged(QDate)), ui->calendarWidget, SLOT(setSelectedDate(QDate)));

    connect(ui->setAlarmTimeButton, SIGNAL(clicked()), this, SLOT(setAlarmTime()));
    connect(ui->resetAlarmButton, SIGNAL(clicked()), this, SLOT(resetAlarmTime()));

    connect(ui->logListWidget->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            ui->logListWidget, SLOT(scrollToBottom())); //Autoscrolling

    connect(ui->logListWidget->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(showLog())); //Show log when first log message arrived

    connect(ui->cleanLogButton, SIGNAL(clicked()), ui->logListWidget, SLOT(clear())); //Clear the log

    Rtc* rtc = Rtc::instance();
    switch (rtc->getSystemTimeSpec())
    {
    case 0:
        rtc->setTimeSpec(Qt::LocalTime); break;
    case 1:
        rtc->setTimeSpec(Qt::UTC); break;
    default:
        {
            QListWidgetItem* item = new QListWidgetItem(ui->logListWidget);
            item->setText(rtc->errorString());
        }
    }

    int fDay =  QApplication::translate("Localization", "7",
                                        "Just enter number of the first day of the "
                                        "week in the field of translation, where 1 is Monday and 7 is Sunday",
                                        QCoreApplication::UnicodeUTF8).toInt();
    ui->calendarWidget->setFirstDayOfWeek(static_cast< Qt::DayOfWeek>(fDay));

    QString transTimeFormatMessage = QApplication::translate("Localization", "yyyy-MM-dd hh:mm",
                                                             "Set your time format or leave blank "
                                                             "to use default format (yyyy-MM-dd hh:mm). "
                                                             "Format description is in project wiki.",
                                                             QCoreApplication::UnicodeUTF8);
    ui->dateTimeEdit->setDisplayFormat(transTimeFormatMessage);

    updateAlarmTime();
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
        ui->calendarWidget->markAlarmDate(alarmTime.date());
        goToAlarmDate();

        ui->stateLabel->setText(tr("%1 Alarm is set %2").arg("<font color=green>").arg("</font>"));
        ui->goToAlarmButton->setEnabled(true);
    }
    else
    {
        ui->calendarWidget->unmarkAlarmDate();
        ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
        if (rtc->error() == Rtc::AlarmIsNotSet)
        {
            ui->stateLabel->setText(tr("%1 Alarm isn't set %2").arg("<font color=red>").arg("</font>"));
            ui->goToAlarmButton->setEnabled(false);
        }
        else
        {
            QListWidgetItem* item = new QListWidgetItem(ui->logListWidget);
            item->setText(rtc->errorString());
        }
    }
}

void MainWindow::goToAlarmDate()
{
    QDate alarmDate = ui->calendarWidget->getMarkedDate();
    ui->calendarWidget->setCurrentPage(alarmDate.year(), alarmDate.month());
}

void MainWindow::goToCurrentMonth()
{
    QDate currentDate = QDate::currentDate();
    ui->calendarWidget->setCurrentPage(currentDate.year(), currentDate.month());
}

void MainWindow::goToToday()
{
    ui->calendarWidget->setSelectedDate(QDate::currentDate());
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    setFocus();
}

void MainWindow::setVisibleLog(bool visible)
{
    //TODO: Animation

    ui->logBox->setVisible(visible);
}

void MainWindow::showLog()
{
    ui->logBox->show();
    if (!ui->logButton->isChecked())
    {
        ui->logButton->toggle();
    }
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

void MainWindow::resetAlarmTime()
{

    Rtc* rtc = Rtc::instance();
    int err = rtc->resetAlarmTime();
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

void MainWindow::createActions()
{
    aboutAct = new QAction(trUtf8("About"), this);
    //aboutAct->setStatusTip(trUtf8("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(trUtf8("About Qt"), this);
    //aboutQtAct->setStatusTip(trUtf8("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    helpMenu = menuBar()->addMenu(trUtf8("Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::about()
{
    QString aboutText = QString::fromUtf8("QRtcAlarm\n\n");
    if (!QString(GIT_COMMIT_HASH).isEmpty())
    {
        aboutText += QString::fromUtf8("Commit hash: %1\n\n").arg(GIT_COMMIT_HASH);
    }

    aboutText += "Copyright (c) 2010 Egor Lynko\n";

    QString transTranslatorNameText = QApplication::translate("Localization", "Egor Lynko",
                                                      "Enter the name of the translator "
                                                      "(i.e. yours) in English",
                                                      QCoreApplication::UnicodeUTF8);
    aboutText += trUtf8("Translator") + QString::fromUtf8(": ");
    aboutText += transTranslatorNameText += QString::fromUtf8("\n\n");

    QMessageBox::about(this, QString::fromUtf8("About"), aboutText);
}

void MainWindow::setFocus()
{
    ui->dateTimeEdit->setFocus();
    ui->dateTimeEdit->setSelectedSection(QDateTimeEdit::HourSection);
}
