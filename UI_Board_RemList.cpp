#include <QDebug>
#include <QListWidgetItem>
#include <QApplication>
#include <QPalette>
#include "UI_Board_RemList.h"
using namespace nsUI_Board_RemList;

clUI_Board_RemList::clUI_Board_RemList(QWidget *parent)
    : QListWidget(parent)
{
    QSizePolicy size_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    size_policy.setHorizontalStretch(2);
    setSizePolicy(size_policy);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // palette
    QPalette palette = QGuiApplication::palette();
    palette.setColor(QPalette::Highlight, QColor("#cccccc"));
    setPalette(palette);

    setStyleSheet("QListWidget::item { selection-color: black }");

    //
    mTimer_blink.setInterval(700);
    m_blink_state = false;
    m_blinks_selected_item = false;

    //
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(On_current_item_changed(QListWidgetItem*,QListWidgetItem*)));
    connect(&mTimer_blink, SIGNAL(timeout()), this, SLOT(On_timer_blink_time_out()));

    // text color, font
    mNormalTextColor = QColor(Qt::black);
    mAlternateTextColor = "#b0601a";
    mNormalFont = QApplication::font().family();
    mNormalFont.setPointSize(12);

    //
    mSoundPlayer = new clSoundPlayer(this);
}

clUI_Board_RemList::~clUI_Board_RemList()
{
    for(auto it=mItemsWithAlarm.begin(); it!=mItemsWithAlarm.end(); it++)
        it.value()->deleteLater();
}

void clUI_Board_RemList::add_reminder_title(const int rem_id, const QString &rem_title,
                                            const clDataElem_ReminderAlarm &alarm_spec)
//Add the reminder if it is not already added.
//Also select the reminder.
//Alarm will not be started.
{
    if(! mReminderIDtoItem.contains(rem_id))
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setData(Qt::DisplayRole, rem_title);
        item->setData(Qt::ForegroundRole, QBrush(mNormalTextColor));
        item->setData(Qt::FontRole, mNormalFont);
        addItem(item);

        mReminderIDtoItem.insert(rem_id, item);

        //
        if(! alarm_spec.is_empty())
        {
            clItemAlarm *item_alarm = new clItemAlarm(alarm_spec, nullptr);
            mItemsWithAlarm.insert(item, item_alarm);

            connect(item_alarm, SIGNAL(start_blinking(nsUI_Board_RemList::clItemAlarm*)),
                    this,         SLOT(start_blinking_item(nsUI_Board_RemList::clItemAlarm*)));
            connect(item_alarm, SIGNAL(stop_blinking(nsUI_Board_RemList::clItemAlarm*)),
                    this,         SLOT(stop_blinking_item(nsUI_Board_RemList::clItemAlarm*)));
            connect(item_alarm, SIGNAL(start_sound(nsUI_Board_RemList::clItemAlarm*)),
                    this,         SLOT(start_playing_sound(nsUI_Board_RemList::clItemAlarm*)));
            connect(item_alarm, SIGNAL(stop_sound(nsUI_Board_RemList::clItemAlarm*)),
                    this,         SLOT(stop_playing_sound(nsUI_Board_RemList::clItemAlarm*)));
            connect(item_alarm, SIGNAL(state_changed(nsUI_Board_RemList::clItemAlarm *, nsUI_Board_RemList::clItemAlarm::enState)),
                    this, SLOT(On_item_alarm_state_changed(nsUI_Board_RemList::clItemAlarm *, nsUI_Board_RemList::clItemAlarm::enState)));
        }
    }

    //
    setCurrentItem(mReminderIDtoItem[rem_id]);
}

void clUI_Board_RemList::remove_reminder_title(const int rem_id)
{
    if(! mReminderIDtoItem.contains(rem_id))
        return;

    QListWidgetItem *item = mReminderIDtoItem.take(rem_id);
    int row = this->row(item);
    Q_ASSERT(row >= 0);
    this->takeItem(row);

    //
    if(mItemsWithAlarm.contains(item))
    {
        mItemsWithAlarm[item]->stop();
        mItemsWithAlarm[item]->deleteLater();
        mItemsWithAlarm.remove(item);
    }

    //
    delete item;
}

bool clUI_Board_RemList::contains_reminder_title(const int rem_id) const
{
    return mReminderIDtoItem.contains(rem_id);
}

void clUI_Board_RemList::update_reminder_title(const int rem_id, const QString &new_title)
{
    Q_ASSERT(mReminderIDtoItem.contains(rem_id));
    QListWidgetItem *item = mReminderIDtoItem[rem_id];
    item->setData(Qt::DisplayRole, new_title);
}

int clUI_Board_RemList::get_current_reminder_id() const
//returns -1 if not found
{
    QListWidgetItem *item = this->currentItem();
    if(item == 0)
        return -1;

    int id = mReminderIDtoItem.key(item, -1);
    Q_ASSERT(id != -1);
    return id;
}

void clUI_Board_RemList::start_alarm_on_reminder(const int rem_id)
//If the alarm is already started, restart it.
{
    Q_ASSERT(mReminderIDtoItem.contains(rem_id));
    QListWidgetItem *item = mReminderIDtoItem[rem_id];

    Q_ASSERT(mItemsWithAlarm.contains(item));
    mItemsWithAlarm[item]->restart();
}

void clUI_Board_RemList::delay_alarm_on_reminder(const int rem_id)
//effective only if the alarm is active
{
    Q_ASSERT(mReminderIDtoItem.contains(rem_id));
    QListWidgetItem *item = mReminderIDtoItem[rem_id];

    Q_ASSERT(mItemsWithAlarm.contains(item));
    mItemsWithAlarm[item]->delay(); //effective only if the item-alarm is active
}

void clUI_Board_RemList::stop_alarm_on_reminder(const int rem_id)
{
    Q_ASSERT(mReminderIDtoItem.contains(rem_id));
    QListWidgetItem *item = mReminderIDtoItem[rem_id];

    Q_ASSERT(mItemsWithAlarm.contains(item));
    mItemsWithAlarm[item]->stop();
}

bool clUI_Board_RemList::alarm_running_on_reminder(const int rem_id) const
//Return true if `rem_id` has alarm and the alarm has started (not yet stopped).
{
    Q_ASSERT(mReminderIDtoItem.contains(rem_id));
    QListWidgetItem *item = mReminderIDtoItem[rem_id];

    if(! mItemsWithAlarm.contains(item))
        return false;
    return mItemsWithAlarm[item]->is_running();
}

void clUI_Board_RemList::On_current_item_changed(QListWidgetItem *item_current,
                                                 QListWidgetItem *item_prev)
{
    Q_UNUSED(item_prev);

    bool blinking = false; //will be whether the current item is blinking
    if(mItemsWithAlarm.contains(item_current))
    {
        if(mItemsWithAlarm[item_current]->is_active())
            blinking = true;
    }

    //
    if(blinking)
    {
        if(! m_blinks_selected_item)
        {
            m_blinks_selected_item = true;
            set_highlighted_text_color(m_blink_state);
        }
    }
    else
    {
        if(m_blinks_selected_item)
        {
            m_blinks_selected_item = false;
            set_highlighted_text_color(false);
        }
    }


    // get alarm state
    enAlarmState alarm_state = NoAlarm;
    if(mItemsWithAlarm.contains(item_current))
    {
        clItemAlarm::enState s = mItemsWithAlarm[item_current]->get_state();
        if(s == clItemAlarm::STOPPED)
            alarm_state = AlarmStopped;
        else if(s == clItemAlarm::ACTIVE)
            alarm_state = AlarmActive;
        else if(s == clItemAlarm::PAUSED)
            alarm_state = AlarmPaused;
    }

    // emit signal
    int id = mReminderIDtoItem.key(item_current, -1);
    emit current_reminder_changed(id, alarm_state);
}

void clUI_Board_RemList::On_timer_blink_time_out()
{
    m_blink_state = !m_blink_state;

    //
    QColor text_color;
    if(m_blink_state)
        text_color.setNamedColor(mAlternateTextColor);
    else
        text_color = mNormalTextColor;

    //
    for(auto it=mBlinkingItems.begin(); it!=mBlinkingItems.end(); it++)
    {
        Q_ASSERT(row(*it) >= 0);
        (*it)->setData(Qt::ForegroundRole, QBrush(text_color));
    }

    //
    if(m_blinks_selected_item)
        set_highlighted_text_color(m_blink_state);
}

void clUI_Board_RemList::start_blinking_item(clItemAlarm *by_item_alarm)
{
    QListWidgetItem *item = mItemsWithAlarm.key(by_item_alarm, nullptr);
    Q_ASSERT(item != nullptr);
    Q_ASSERT(row(item) >= 0);

    //
    mBlinkingItems.insert(item);

    // set bold font for `item`
    QFont font = mNormalFont;
    font.setBold(true);
    item->setData(Qt::FontRole, font);

    //
    if(currentItem() == item) //(`item` is selected)
        m_blinks_selected_item = true;

    //
    if(! mTimer_blink.isActive())
        mTimer_blink.start();
}

void clUI_Board_RemList::stop_blinking_item(clItemAlarm *by_item_alarm)
{
    QListWidgetItem *item = mItemsWithAlarm.key(by_item_alarm, nullptr);
    Q_ASSERT(item != nullptr);
    Q_ASSERT(row(item) >= 0);

    //
    mBlinkingItems.remove(item);

    // set normal text font and color for `item`
    item->setData(Qt::FontRole, mNormalFont);
    item->setData(Qt::ForegroundRole, QBrush(mNormalTextColor));

    //
    if(currentItem() == item)
    {
        m_blinks_selected_item = false;
        set_highlighted_text_color(false);
    }

    //
    if(mBlinkingItems.isEmpty())
        mTimer_blink.stop();
}

void clUI_Board_RemList::start_playing_sound(clItemAlarm *by_item_alarm)
{
    QListWidgetItem *item = mItemsWithAlarm.key(by_item_alarm, nullptr);
    Q_ASSERT(item != nullptr);

    mSoundPlayingItems.insert(item);

    if(! mSoundPlayer->is_playing())
        mSoundPlayer->play();
}

void clUI_Board_RemList::stop_playing_sound(clItemAlarm *by_item_alarm)
{
    QListWidgetItem *item = mItemsWithAlarm.key(by_item_alarm, nullptr);
    Q_ASSERT(item != nullptr);

    mSoundPlayingItems.remove(item);

    if(mSoundPlayingItems.isEmpty())
        mSoundPlayer->stop();
}

void clUI_Board_RemList::On_item_alarm_state_changed(clItemAlarm *by_item_alarm,
                                                     clItemAlarm::enState new_state)
{
    QListWidgetItem *item = mItemsWithAlarm.key(by_item_alarm, nullptr);
    Q_ASSERT(item != nullptr);

    if(item == currentItem())
    {
        enAlarmState alarm_state;
        if(new_state == clItemAlarm::STOPPED)
            alarm_state = AlarmStopped;
        else if(new_state == clItemAlarm::ACTIVE)
            alarm_state = AlarmActive;
        else if(new_state == clItemAlarm::PAUSED)
            alarm_state = AlarmPaused;
        else
            Q_ASSERT(false);

        emit current_reminder_alarm_state_changed(mReminderIDtoItem.key(item), alarm_state);
    }
}

void clUI_Board_RemList::set_highlighted_text_color(bool alternate_color)
{
    if(alternate_color)
        setStyleSheet("QListWidget::item { selection-color: "+mAlternateTextColor+" }");
    else
        setStyleSheet("QListWidget::item { selection-color: black }");
}

//////////////////////////////////////////////////////////////////////////////////////////////

clSoundPlayer::clSoundPlayer(QObject *parent)
    : QObject(parent)
{
    mGap = 1800; //msec

    //
    mTimer.setSingleShot(true);
    mTimer.setInterval(mGap);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(On_timer_time_out()));

    //
    mSound.setSource(QUrl::fromLocalFile(":/sounds/sounds/ding.wav"));
    mSound.setLoopCount(1);
    connect(&mSound, SIGNAL(playingChanged()), this, SLOT(On_sound_playing_changed()));

}

void clSoundPlayer::play()
//restart if already playing
{
    stop();
    mSound.play();
}

void clSoundPlayer::stop()
{
    mSound.stop();
    mTimer.stop();
}

void clSoundPlayer::On_timer_time_out()
{
    mSound.play();
}

void clSoundPlayer::On_sound_playing_changed()
{
    if(! mSound.isPlaying())
        mTimer.start();
}

bool clSoundPlayer::is_playing() const
{
    return mSound.isPlaying() || mTimer.isActive();
}
