#-------------------------------------------------
#
# Project created by QtCreator 2018-01-26T11:56:43
#
#-------------------------------------------------

QT       += core gui
QT       += xml
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyReminder20
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        UI_MainWindow.cpp \
    Control.cpp \
    DataManager.cpp \
    AlarmClockService.cpp \
    DataElem_Alarm.cpp \
    DataElem_ButtonSet.cpp \
    DataElem_DatePattern.cpp \
    DataElem_GEvent.cpp \
    DataElem_ScheduleStatus.cpp \
    DataElem_TimeCollection.cpp \
    DataElem_TimeRangeCollection.cpp \
    DataElem_TimeSequence.cpp \
    UI_Dialog_TestingDatePattern.cpp \
    Reminder.cpp \
    ReminderSpec.cpp \
    RemSpec_Abstract.cpp \
    RemSpec_DateSetting.cpp \
    RemSpec_GEventDurationBinding.cpp \
    RemSpec_GEventTrigger.cpp \
    RemSpec_SituationBinding.cpp \
    RemSpec_TimeRangeBinding.cpp \
    RemSpec_TimeRangeFilter.cpp \
    RemSpec_TimeTrigger.cpp \
    UI_AllReminders.cpp \
    UI_AllReminders_Dialog_AddCustomButtonSet.cpp \
    UI_AllReminders_Dialog_EditSpec.cpp \
    UI_AllReminders_Dialog_EditTags.cpp \
    UI_Board.cpp \
    UI_Board_RemList.cpp \
    UI_Board_RemList_ItemAlarm.cpp \
    UI_DayPlan.cpp \
    UI_DayPlan_Dialog_Schedule.cpp \
    UI_Debug_ScheduledActions.cpp \
    UI_MyCalendar.cpp \
    Util_AlarmClock.cpp \
    Util_Button.cpp \
    Util_DateRange.cpp \
    Util_DateSet.cpp \
    Util_HrMinRange.cpp \
    utility_date.cpp \
    utility_DOM.cpp \
    utility_file.cpp \
    utility_general.cpp \
    utility_math.cpp \
    utility_time.cpp \
    utility_widgets.cpp \
    Util_MessageClock.cpp \
    Util_TimeRange.cpp \
    FileReadWrite.cpp \
    UI_Dialog_SpecInputHelper.cpp \
    UI_Dialog_SpecElementInsert.cpp \
    UI_Dialog_CalendarDatePicker.cpp \
    UI_Board_ButtonSetsView.cpp

HEADERS += \
        UI_MainWindow.h \
    Control.h \
    DataManager.h \
    AlarmClockService.h \
    DataElem_Alarm.h \
    DataElem_ButtonSet.h \
    DataElem_DatePattern.h \
    DataElem_GEvent.h \
    DataElem_ScheduleStatus.h \
    DataElem_TimeCollection.h \
    DataElem_TimeRangeCollection.h \
    DataElem_TimeSequence.h \
    UI_Dialog_TestingDatePattern.h \
    Reminder.h \
    ReminderSpec.h \
    RemSpec_Abstract.h \
    RemSpec_DateSetting.h \
    RemSpec_GEventDurationBinding.h \
    RemSpec_GEventTrigger.h \
    RemSpec_SituationBinding.h \
    RemSpec_TimeRangeBinding.h \
    RemSpec_TimeRangeFilter.h \
    RemSpec_TimeTrigger.h \
    UI_AllReminders.h \
    UI_AllReminders_Dialog_AddCustomButtonSet.h \
    UI_AllReminders_Dialog_EditSpec.h \
    UI_AllReminders_Dialog_EditTags.h \
    UI_Board.h \
    UI_Board_RemList.h \
    UI_Board_RemList_ItemAlarm.h \
    UI_DayPlan.h \
    UI_DayPlan_Dialog_Schedule.h \
    UI_Debug_ScheduledActions.h \
    UI_MyCalendar.h \
    Util_AlarmClock.h \
    Util_Button.h \
    Util_DateRange.h \
    Util_DateSet.h \
    Util_HrMinRange.h \
    utility_date.h \
    utility_DOM.h \
    utility_file.h \
    utility_general.h \
    utility_math.h \
    utility_time.h \
    utility_widgets.h \
    Util_MessageClock.h \
    Util_TimeRange.h \
    UI_Dialog_SpecInputHelper.h \
    UI_Dialog_SpecElementInsert.h \
    UI_Dialog_CalendarDatePicker.h \
    UI_Board_ButtonSetsView.h

FORMS += \
        UI_MainWindow.ui \
    UI_Dialog_TestingDatePattern.ui \
    UI_AllReminders.ui \
    UI_AllReminders_Dialog_AddCustomButtonSet.ui \
    UI_AllReminders_Dialog_EditSpec.ui \
    UI_AllReminders_Dialog_EditTags.ui \
    UI_Board.ui \
    UI_DayPlan.ui \
    UI_DayPlan_Dialog_Schedule.ui \
    UI_Debug_ScheduledActions.ui \
    UI_MyCalendar.ui \
    UI_Dialog_SpecInputHelper.ui \
    UI_Dialog_SpecElementInsert.ui \
    UI_Dialog_CalendarDatePicker.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    Todos.txt
