#pragma once
#include <Process/Commands/SetControlValue.hpp>
#include <Process/Dataflow/ControlWidgets.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/Dataflow/WidgetInlets.hpp>
#include <State/Value.hpp>

#include <score/command/Dispatchers/CommandDispatcher.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/widgets/ControlWidgets.hpp>
#include <score/widgets/DoubleSlider.hpp>
#include <score/graphics/GraphicWidgets.hpp>
#include <score/widgets/MarginLess.hpp>
#include <score/widgets/SignalUtils.hpp>
#include <score/widgets/TextLabel.hpp>

#include <ossia/network/domain/domain.hpp>

#include <QApplication>
#include <QCheckBox>
#include <QFormLayout>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>

#include <Engine/Node/Port.hpp>
#include <score_plugin_engine_export.h>
namespace Control
{
struct FloatSlider final : ossia::safe_nodes::control_in,
                           WidgetFactory::FloatSlider
{
  static const constexpr bool must_validate = false;
  using type = float;
  const float min{};
  const float max{};
  const float init{};

  template <std::size_t N>
  constexpr FloatSlider(const char (&name)[N], float v1, float v2, float v3)
      : ossia::safe_nodes::control_in{name}, min{v1}, max{v2}, init{v3}
  {
  }

  auto getMin() const
  {
    return min;
  }
  auto getMax() const
  {
    return max;
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::FloatSlider{
        min, max,   init, QString::fromUtf8(name.data(), name.size()),
        id,  parent};
  }

  float fromValue(const ossia::value& v) const
  {
    return ossia::convert<float>(v);
  }
  ossia::value toValue(float v) const
  {
    return v;
  }
};

struct LogFloatSlider final : ossia::safe_nodes::control_in,
                              WidgetFactory::LogFloatSlider
{
  static const constexpr bool must_validate = false;
  using type = float;
  const float min{};
  const float max{};
  const float init{};

  template <std::size_t N>
  constexpr LogFloatSlider(const char (&name)[N], float v1, float v2, float v3)
      : ossia::safe_nodes::control_in{name}, min{v1}, max{v2}, init{v3}
  {
  }

  auto getMin() const
  {
    return min;
  }
  auto getMax() const
  {
    return max;
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::LogFloatSlider{
        min, max,   init, QString::fromUtf8(name.data(), name.size()),
        id,  parent};
  }

  float fromValue(const ossia::value& v) const
  {
    return ossia::convert<float>(v);
  }
  ossia::value toValue(float v) const
  {
    return v;
  }
};

struct IntSlider final : ossia::safe_nodes::control_in,
                         WidgetFactory::IntSlider
{
  using type = int;
  const int min{};
  const int max{};
  const int init{};

  static const constexpr bool must_validate = false;
  template <std::size_t N>
  constexpr IntSlider(const char (&name)[N], int v1, int v2, int v3)
      : ossia::safe_nodes::control_in{name}, min{v1}, max{v2}, init{v3}
  {
  }

  int fromValue(const ossia::value& v) const
  {
    return ossia::convert<int>(v);
  }
  ossia::value toValue(int v) const
  {
    return v;
  }

  auto getMin() const
  {
    return min;
  }
  auto getMax() const
  {
    return max;
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::IntSlider{
        min, max,   init, QString::fromUtf8(name.data(), name.size()),
        id,  parent};
  }
};

struct IntSpinBox final : ossia::safe_nodes::control_in,
                          WidgetFactory::IntSpinBox
{
  static const constexpr bool must_validate = false;
  using type = int;
  const int min{};
  const int max{};
  const int init{};

  int fromValue(const ossia::value& v) const
  {
    return ossia::convert<int>(v);
  }
  ossia::value toValue(int v) const
  {
    return v;
  }

  template <std::size_t N>
  constexpr IntSpinBox(const char (&name)[N], int v1, int v2, int v3)
      : ossia::safe_nodes::control_in{name}, min{v1}, max{v2}, init{v3}
  {
  }

  auto getMin() const
  {
    return min;
  }
  auto getMax() const
  {
    return max;
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::IntSpinBox{
        min, max,   init, QString::fromUtf8(name.data(), name.size()),
        id,  parent};
  }
};
struct Toggle final : ossia::safe_nodes::control_in, WidgetFactory::Toggle
{
  static const constexpr bool must_validate = false;
  template <std::size_t N>
  constexpr Toggle(const char (&name)[N], bool v1)
      : ossia::safe_nodes::control_in{name}, init{v1}
  {
  }

  using type = bool;
  const bool init{};
  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::Toggle{
        init, QString::fromUtf8(name.data(), name.size()), id, parent};
  }

  bool fromValue(const ossia::value& v) const
  {
    return ossia::convert<bool>(v);
  }
  ossia::value toValue(bool v) const
  {
    return v;
  }
};

struct ChooserToggle final : ossia::safe_nodes::control_in,
                             WidgetFactory::ChooserToggle
{
  static const constexpr bool must_validate = false;
  template <std::size_t N>
  constexpr ChooserToggle(
      const char (&name)[N], std::array<const char*, 2> alt, bool v1)
      : ossia::safe_nodes::control_in{name}, alternatives{alt}, init{v1}
  {
  }
  using type = bool;
  std::array<const char*, 2> alternatives;
  const bool init{};

  bool fromValue(const ossia::value& v) const
  {
    return ossia::convert<bool>(v);
  }
  ossia::value toValue(bool v) const
  {
    return v;
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::ChooserToggle{
        {alternatives[0], alternatives[1]},
        init,
        QString::fromUtf8(name.data(), name.size()),
        id,
        parent};
  }
};
struct LineEdit final : ossia::safe_nodes::control_in, WidgetFactory::LineEdit
{
  static const constexpr bool must_validate = false;
  template <std::size_t N, std::size_t M>
  constexpr LineEdit(const char (&name)[N], const char (&init)[M])
      : ossia::safe_nodes::control_in{name}, init{init, M - 1}
  {
  }

  std::string fromValue(const ossia::value& v) const
  {
    return ossia::convert<std::string>(v);
  }
  ossia::value toValue(std::string v) const
  {
    return ossia::value{std::move(v)};
  }

  using type = std::string;
  const QLatin1Literal init{};
  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::LineEdit{
        init, QString::fromUtf8(name.data(), name.size()), id, parent};
  }
};

template <typename T, std::size_t N>
struct ComboBox final : ossia::safe_nodes::control_in, WidgetFactory::ComboBox
{
  static const constexpr bool must_validate = false;
  using type = T;
  const std::size_t init{};
  const std::array<std::pair<const char*, T>, N> values;

  static constexpr auto count()
  {
    return N;
  }
  template <std::size_t M, typename Arr>
  constexpr ComboBox(const char (&name)[M], std::size_t in, Arr arr)
      : ossia::safe_nodes::control_in{name}, init{in}, values{arr}
  {
  }

  const auto& getValues() const
  {
    return values;
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    auto p = new Process::ControlInlet(id, parent);
    p->type = Process::PortType::Message;
    p->setValue(values[init].second);
    p->setCustomData(QString::fromUtf8(name.data(), name.size()));
    return p;
  }

  T fromValue(const ossia::value& v) const
  {
    return ossia::convert<T>(v);
  }
  ossia::value toValue(T v) const
  {
    return ossia::value{std::move(v)};
  }
};

template <typename ArrT>
struct EnumBase : ossia::safe_nodes::control_in, WidgetFactory::Enum
{
  using type = std::string;
  const std::size_t init{};
  const ArrT values;

  const auto& getValues() const
  {
    return values;
  }

  template <std::size_t N1>
  constexpr EnumBase(const char (&name)[N1], std::size_t i, const ArrT& v)
      : ossia::safe_nodes::control_in{name}, init{i}, values{v}
  {
  }

  ossia::value toValue(std::string v) const
  {
    return ossia::value{std::move(v)};
  }

  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::Enum{
        ossia::flat_set<std::string>(values.begin(), values.end()),
        values[init], QString::fromUtf8(name.data(), name.size()), id, parent};
  }
};

template <typename ArrT>
struct Enum final : EnumBase<ArrT>
{
  using EnumBase<ArrT>::EnumBase;
  static const constexpr bool must_validate = true;
  auto fromValue(const ossia::value& v) const
  {
    auto t = v.target<std::string>();
    if (t)
    {
      if (auto it = ossia::find(this->values, *t); it != this->values.end())
      {
        return ossia::optional<std::string>{*t};
      }
    }
    return ossia::optional<std::string>{};
  }
};

template <typename ArrT>
struct UnvalidatedEnum final : EnumBase<ArrT>
{
  using EnumBase<ArrT>::EnumBase;
  static const constexpr bool must_validate = false;
  auto fromValue(const ossia::value& v) const
  {
    auto t = v.target<std::string>();
    if (t)
      return *t;
    return std::string{};
  }
};

struct TimeSignatureChooser final : ossia::safe_nodes::control_in,
                                    WidgetFactory::TimeSignatureChooser
{
  static const constexpr bool must_validate = true;
  using type = time_signature;
  const std::string_view init;
  template <std::size_t M, std::size_t N>
  constexpr TimeSignatureChooser(const char (&name)[M], const char (&in)[N])
      : ossia::safe_nodes::control_in{name}, init{in, N}
  {
  }

  ossia::value toValue(time_signature v) const
  {
    std::string s;
    s.reserve(8);
    s += std::to_string(v.first);
    s += '/';
    s += std::to_string(v.second);
    return ossia::value{std::move(s)};
  }

  optional<time_signature> fromValue(const ossia::value& v) const
  {
    if (auto str = v.target<std::string>())
    {
      return get_time_signature(*str);
    }
    return ossia::none;
  }
  auto create_inlet(Id<Process::Port> id, QObject* parent) const
  {
    return new Process::TimeSignatureChooser{
        init.data(), QString::fromUtf8(name.data(), name.size()), id, parent};
  }
};

template <typename T1, typename T2>
constexpr auto make_enum(const T1& t1, std::size_t s, const T2& t2)
{
  return Control::Enum<T2>(t1, s, t2);
}
template <typename T1, typename T2>
constexpr auto make_unvalidated_enum(const T1& t1, std::size_t s, const T2& t2)
{
  return Control::UnvalidatedEnum<T2>(t1, s, t2);
}
/*
template<std::size_t N1, std::size_t N2>
Enum(const char (&name)[N1], std::size_t i, const std::array<const char*, N2>&
v) -> Enum<std::array<const char*, N2>>;
*/

// TODO RGBAEdit
struct RGBAEdit final : ossia::safe_nodes::control_in, WidgetFactory::RGBAEdit
{
  static const constexpr bool must_validate = false;
  using type = std::array<float, 4>;
  std::array<float, 4> init{};
};

// TODO XYZEdit
struct XYZEdit final : ossia::safe_nodes::control_in, WidgetFactory::XYZEdit
{
  static const constexpr bool must_validate = false;
  using type = std::array<float, 3>;
  std::array<float, 3> init{};
};
}
