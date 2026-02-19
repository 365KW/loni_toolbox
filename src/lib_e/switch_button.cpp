//
// Created by kyyou on 2026/2/18.
//

#include "switch_button.h"


#include <QPainter>
#include <QDebug>

SwitchButton::SwitchButton(QWidget *parent)
    : QWidget(parent)
    , animation_(new QPropertyAnimation(this, "sliderPosition", this)) {
    setFixedSize(switch_width_, switch_height_);
    setCursor(Qt::PointingHandCursor);

    animation_->setDuration(150);
    animation_->setEasingCurve(QEasingCurve::InOutCubic);
}

bool SwitchButton::isChecked() const {
    return checked_;
}

void SwitchButton::setChecked(bool checked) {
    if (checked_ != checked) {
        checked_ = checked;
        updateSliderPosition();
        emit toggled(checked_);
        emit stateChanged(checked_ ? Qt::Checked : Qt::Unchecked);
    }
}

void SwitchButton::setEnabled(bool enabled) {
    enabled_ = enabled;
    setCursor(enabled_ ? Qt::PointingHandCursor : Qt::ArrowCursor);
    update();
}

void SwitchButton::setColors(const QColor &bgOff, const QColor &bgOn,
                             const QColor &sliderOff, const QColor &sliderOn) {
    bg_color_off_ = bgOff;
    bg_color_on_ = bgOn;
    slider_color_off_ = sliderOff;
    slider_color_on_ = sliderOn;
    update();
}

void SwitchButton::setSize(int width, int height) {
    switch_width_ = width;
    switch_height_ = height;
    setFixedSize(switch_width_, switch_height_);
    update();
}

void SwitchButton::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal opacity = enabled_ ? 1.0 : 0.5;
    painter.setOpacity(opacity);

    // 绘制背景
    QColor bgColor = checked_ ? bg_color_on_ : bg_color_off_;
    if (!enabled_) {
        bgColor = QColor(200, 200, 200);
    }
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), switch_height_ / 2.0, switch_height_ / 2.0);

    // 绘制阴影效果
    if (enabled_ && hovered_) {
        QColor shadowColor(0, 0, 0, 30);
        painter.setBrush(shadowColor);
        painter.drawRoundedRect(rect(), switch_height_ / 2.0, switch_height_ / 2.0);
    }

    // 绘制滑块
    QColor sliderColor = checked_ ? slider_color_on_ : slider_color_off_;
    painter.setBrush(sliderColor);

    int sliderDiameter = switch_height_ - 2 * slider_margin_;
    int sliderX = slider_margin_ + static_cast<int>(slider_position_ * (switch_width_ - sliderDiameter - 2 * slider_margin_));
    int sliderY = slider_margin_;

    // 绘制滑块阴影
    if (enabled_) {
        painter.setPen(Qt::NoPen);
        QColor shadow(0, 0, 0, 40);
        painter.setBrush(shadow);
        painter.drawEllipse(sliderX, sliderY + 1, sliderDiameter, sliderDiameter);
    }

    // 绘制滑块本体
    painter.setBrush(sliderColor);
    painter.drawEllipse(sliderX, sliderY, sliderDiameter, sliderDiameter);
}

void SwitchButton::mousePressEvent(QMouseEvent *event) {
    if (enabled_ && event->button() == Qt::LeftButton) {
        pressed_ = true;
        update();
    }
    QWidget::mousePressEvent(event);
}

void SwitchButton::mouseReleaseEvent(QMouseEvent *event) {
    if (enabled_ && event->button() == Qt::LeftButton) {
        pressed_ = false;
        if (rect().contains(event->pos())) {
            setChecked(!checked_);
        }
        update();
    }
    QWidget::mouseReleaseEvent(event);
}

void SwitchButton::enterEvent(QEnterEvent *event) {
    hovered_ = true;
    update();
    QWidget::enterEvent(event);
}

void SwitchButton::leaveEvent(QEvent *event) {
    hovered_ = false;
    pressed_ = false;
    update();
    QWidget::leaveEvent(event);
}

void SwitchButton::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateSliderPosition();
}

qreal SwitchButton::sliderPosition() const {
    return slider_position_;
}

void SwitchButton::setSliderPosition(qreal position) {
    slider_position_ = position;
    update();
}

void SwitchButton::updateSliderPosition() {
    if (animation_->state() == QPropertyAnimation::Running) {
        animation_->stop();
    }
    animation_->setStartValue(slider_position_);
    animation_->setEndValue(checked_ ? 1.0 : 0.0);
    animation_->start();
}
