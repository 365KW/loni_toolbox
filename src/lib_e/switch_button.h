#ifndef LEARNING_CPP_SWITCH_BUTTON_H
#define LEARNING_CPP_SWITCH_BUTTON_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QMouseEvent>

class SwitchButton : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal sliderPosition READ sliderPosition WRITE setSliderPosition)

public:
    explicit SwitchButton(QWidget *parent = nullptr);
    ~SwitchButton() override = default;

    bool isChecked() const;
    void setChecked(bool checked);

    void setEnabled(bool enabled);

    void setColors(const QColor &bgOff, const QColor &bgOn,
                   const QColor &sliderOff, const QColor &sliderOn);
    void setSize(int width, int height);

signals:
    void toggled(bool checked);
    void stateChanged(int state);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    qreal sliderPosition() const;
    void setSliderPosition(qreal position);
    void updateSliderPosition();

    bool checked_ = false;
    bool enabled_ = true;
    bool hovered_ = false;
    bool pressed_ = false;

    qreal slider_position_ = 0.0;

    int switch_width_ = 50;
    int switch_height_ = 26;
    int slider_margin_ = 3;

    QColor bg_color_off_ = QColor(200, 200, 200);
    QColor bg_color_on_ = QColor(76, 175, 80);
    QColor slider_color_off_ = QColor(255, 255, 255);
    QColor slider_color_on_ = QColor(255, 255, 255);

    QPropertyAnimation *animation_ = nullptr;
};

#endif //LEARNING_CPP_SWITCH_BUTTON_H
