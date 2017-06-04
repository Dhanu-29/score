#pragma once
#include <ossia/editor/automation/automation.hpp>
#include <ossia/editor/value/value.hpp>
#include <Automation/Color/GradientAutomModel.hpp>
#include <Engine/Executor/ProcessComponent.hpp>
#include <ossia/editor/scenario/time_constraint.hpp>
#include <ossia/editor/curve/curve_segment/easing.hpp>
#include <memory>
namespace ossia
{
class curve_abstract;

class color_automation : public ossia::time_process
{
  public:
    using grad_type = boost::container::flat_map<double, ossia::hsv>;

    color_automation()
    {

    }

    void set_destination(ossia::Destination a)
    {
      m_address = std::move(a);
    }

    void set_gradient(grad_type t)
    {
      m_data = t;
    }

    ossia::state_element offset(ossia::time_value) override
    {
      return {};
    }

    ossia::state_element state() override
    {
      if(m_address)
      {
        auto& addr = *m_address;

        switch(m_data.size())
        {
          case 0:
            return ossia::message{addr, ossia::vec4f{0., 0., 0., 0.}};
          case 1:
            return ossia::message{addr, m_data.begin()->second.dataspace_value};
        }

        auto t = parent()->get_date() / parent()->get_nominal_duration();
        auto it_next = m_data.lower_bound(t);
        // Before start
        if(it_next == m_data.begin())
          return ossia::message{addr, m_data.begin()->second.dataspace_value};

        // past end
        if(it_next == m_data.end())
          return ossia::message{addr, m_data.rbegin()->second.dataspace_value};

        auto it_prev = it_next;
        --it_prev;

        // Interpolate in hsv domain
        ossia::hsv prev{it_prev->second};
        ossia::hsv next{it_next->second};

        auto coeff = (t - it_prev->first) / (it_next->first - it_prev->first);

        ossia::hsv res;
        ossia::easing::ease e{};
        res.dataspace_value = ossia::make_vec(
              e(prev.dataspace_value[0], next.dataspace_value[0], coeff),
            e(prev.dataspace_value[1], next.dataspace_value[1], coeff),
            e(prev.dataspace_value[2], next.dataspace_value[2], coeff)
            );
        ossia::rgb final{res};
        return ossia::message{addr, final.dataspace_value};
      }
      return {};
    }

    void start() override
    {
    }
    void stop() override
    {
    }
    void pause() override
    {
    }
    void resume() override
    {
    }

  private:
    optional<ossia::Destination> m_address;
    grad_type m_data;
};
}

namespace Device
{
class DeviceList;
}

namespace Gradient
{
namespace RecreateOnPlay
{
class Component final
    : public ::Engine::Execution::
    ProcessComponent_T<Gradient::ProcessModel, ossia::color_automation>
{
    COMPONENT_METADATA("553f99e0-a12a-4a1a-a070-63f198908c6d")
    public:
      Component(
        ::Engine::Execution::ConstraintComponent& parentConstraint,
        Gradient::ProcessModel& element,
        const ::Engine::Execution::Context& ctx,
        const Id<iscore::Component>& id,
        QObject* parent);

  private:
    void recompute();

};
using ComponentFactory
= ::Engine::Execution::ProcessComponentFactory_T<Component>;
}
}


ISCORE_CONCRETE_COMPONENT_FACTORY(
    Engine::Execution::ProcessComponentFactory,
    Gradient::RecreateOnPlay::ComponentFactory)