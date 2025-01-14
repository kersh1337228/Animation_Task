#ifndef EASING_FUNCTION_HPP
#define EASING_FUNCTION_HPP

#include <QListWidget>
#include <QPainter>
#include <QtCharts/QtCharts>
#include <ranges>

const qreal pi = std::acos(-1);

class EasingFunction final : public QWidget, public QListWidgetItem {
    Q_OBJECT
public:
    explicit EasingFunction(
        QEasingCurve::Type type,
        QWidget* parent = nullptr
    ) : type_(type), QWidget(parent), QListWidgetItem() {
        setGeometry({0, 0, kMiniatureSize_, kMiniatureSize_});
        constexpr int listGap = 2;
        setSizeHint({kMiniatureSize_, kMiniatureSize_ + listGap});
        auto [name, function] = functionMap_[type];
        constexpr int size = static_cast<int>(1 / kStep_);
        constexpr qreal curveScale = 0.75;
        QList<QPointF> curve{size};
        std::ranges::copy(std::views::iota(0, size)
        | std::views::transform([&function](auto&& i){
            return QPointF(std::forward<decltype(i)>(i) * kStep_ * kMiniatureSize_,
                           curveScale * kMiniatureSize_ * (
                    1 - function(std::forward<decltype(i)>(i) * kStep_)
                ) + 0.5 * (1 - curveScale) * kMiniatureSize_);
        }), curve.begin());
        functionCurve_ = QPolygonF(curve);
        setText("\t\t" + name);
    }
public:
    static decltype(auto) GetSupportedTypes() {
        return std::views::keys(functionMap_);
    }
    [[nodiscard]] QEasingCurve::Type GetType() const {
        return type_;
    }
    [[nodiscard]] QPropertyAnimation::KeyValues GetKeyFrames(
        std::function<QVariant(qreal, qreal)> proj
    ) const {
        constexpr int size = static_cast<int>(1 / kStep_);
        QPropertyAnimation::KeyValues keyFrames{size};
        auto [name, function] = functionMap_[type_];
        std::ranges::copy(std::views::iota(0, size)
        | std::views::transform([&function, &proj](auto&& i) -> QPropertyAnimation::KeyValue {
            return {std::forward<decltype(i)>(i) * kStep_,
                    proj(std::forward<decltype(i)>(i) * kStep_,
                         function(std::forward<decltype(i)>(i) * kStep_))};
        }), keyFrames.begin());
        return keyFrames;
    }
private:
    void paintEvent(QPaintEvent *event) final {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing);
        painter.drawPolyline(functionCurve_);
    }
private:
    QPolygonF functionCurve_;
    QEasingCurve::Type type_ = QEasingCurve::Linear;
    static constexpr int kMiniatureSize_ = 100;
    static constexpr qreal kStep_ = 0.01;
    static constexpr qreal kThreshold_ = 0.5;
    static inline std::unordered_map<
        QEasingCurve::Type, std::pair<QString, std::function<qreal(qreal)>>
    > functionMap_ {
        {QEasingCurve::Linear, {"Linear", [][[nodiscard]](qreal x) noexcept {
            return x;
        }}},
        {QEasingCurve::InOutSine, {"InOutSine", [][[nodiscard]](qreal x) noexcept {
            return kThreshold_ * (1 - std::cos(pi * x));
        }}},
        {QEasingCurve::InOutQuad, {"InOutQuad", [][[nodiscard]](qreal x) noexcept {
            return x < kThreshold_ ? 2 * x * x : 1 - kThreshold_ * std::pow(-2 * x + 2, 2);
        }}},
        {QEasingCurve::InOutCirc, {"InOutCirc", [][[nodiscard]](qreal x) noexcept {
            return x < kThreshold_
                ? kThreshold_ * (1 - std::sqrt(1 - std::pow(2 * x, 2)))
                : kThreshold_ * (std::sqrt(1 - std::pow(-2 * x + 2, 2)) + 1);
        }}},
        {QEasingCurve::InOutElastic, {"InOutElastic", [][[nodiscard]](qreal x) noexcept {
            return x == 0 ? 0 : x == 1 ? 1 : x < kThreshold_
                ? -kThreshold_ * std::pow(2, 20 * x - 10) * std::sin((20 * x - 11.125) * (2 * pi) / 4.5)
                : kThreshold_ * std::pow(2, -20 * x + 10) * std::sin((20 * x - 11.125) * (2 * pi) / 4.5) + 1;
        }}}
    };
};

#endif