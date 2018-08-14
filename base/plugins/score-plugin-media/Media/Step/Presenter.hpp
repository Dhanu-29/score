#pragma once
#include <ossia/detail/math.hpp>

#include <Media/Step/Commands.hpp>
#include <Media/Step/View.hpp>
#include <Process/Focus/FocusDispatcher.hpp>
#include <Process/LayerPresenter.hpp>
#include <score/command/Dispatchers/SingleOngoingCommandDispatcher.hpp>

namespace Media
{
namespace Step
{
class Model;
class Presenter final : public Process::LayerPresenter
{
public:
  explicit Presenter(
      const Process::ProcessModel& model,
      View* view,
      const Process::ProcessPresenterContext& ctx,
      QObject* parent)
      : LayerPresenter{ctx, parent}
      , m_layer{model}
      , m_view{view}
      , m_disp{m_context.context.commandStack}
  {
    putToFront();
    auto& m = static_cast<const Step::Model&>(model);

    connect(view, &View::pressed, this, [&] {
      m_context.context.focusDispatcher.focus(this);
    });

    connect(view, &View::change, this, [&](std::size_t num, float v) {
      auto vec = m.steps();
      if (num > vec.size())
      {
        vec.resize(num, 0.5f);
      }
      v = ossia::clamp(v, 0.f, 1.f);
      vec[num] = v;

      m_disp.submitCommand(m, std::move(vec));
    });

    connect(view, &View::released, this, [&] { m_disp.commit(); });

    connect(
        m_view, &View::askContextMenu, this, &Presenter::contextMenuRequested);

    con(m, &Step::Model::stepsChanged, this, [&] { m_view->update(); });
    con(m, &Step::Model::stepCountChanged, this, [&] { m_view->update(); });
    con(m, &Step::Model::stepDurationChanged, this,
        [&] { on_zoomRatioChanged(m_ratio); });

    view->m_model = &m;
  }

  void setWidth(qreal val) override
  {
    m_view->setWidth(val);
  }
  void setHeight(qreal val) override
  {
    m_view->setHeight(val);
  }

  void putToFront() override
  {
    m_view->setVisible(true);
  }

  void putBehind() override
  {
    m_view->setVisible(false);
  }

  void on_zoomRatioChanged(ZoomRatio r) override
  {
    m_ratio = r;
    auto& m = static_cast<const Step::Model&>(m_layer);
    auto v = TimeVal::fromMsecs(m.stepDuration() / 44.1).toPixels(r);
    m_view->setBarWidth(v);
  }

  void parentGeometryChanged() override
  {
  }

  const Process::ProcessModel& model() const override
  {
    return m_layer;
  }
  const Id<Process::ProcessModel>& modelId() const override
  {
    return m_layer.id();
  }

private:
  const Process::ProcessModel& m_layer;
  View* m_view{};
  SingleOngoingCommandDispatcher<Media::ChangeSteps> m_disp;
  ZoomRatio m_ratio{};
};
}
}
