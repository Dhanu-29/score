// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ScenarioGlobalCommandManager.hpp"

#include <Scenario/Commands/ClearSelection.hpp>
#include <Scenario/Commands/Interval/RemoveProcessFromInterval.hpp>
#include <Scenario/Commands/Scenario/Deletions/ClearInterval.hpp>
#include <Scenario/Commands/Scenario/Deletions/ClearState.hpp>
#include <Scenario/Commands/Scenario/Deletions/RemoveSelection.hpp>
#include <Scenario/Commands/Scenario/Merge/MergeEvents.hpp>
#include <Scenario/Commands/Scenario/Merge/MergeTimeSyncs.hpp>
#include <Scenario/Document/BaseScenario/BaseScenario.hpp>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/State/ItemModel/MessageItemModel.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <Scenario/Document/TimeSync/TimeSyncModel.hpp>
#include <Scenario/Process/Algorithms/Accessors.hpp>
#include <Scenario/Process/ScenarioModel.hpp>

#include <score/command/Dispatchers/CommandDispatcher.hpp>
#include <score/command/Dispatchers/MacroCommandDispatcher.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/model/EntityMap.hpp>
#include <score/model/IdentifiedObject.hpp>
#include <score/selection/Selection.hpp>
#include <score/selection/SelectionDispatcher.hpp>
#include <score/selection/SelectionStack.hpp>
#include <score/tools/std/Optional.hpp>

#include <QDebug>

#include <algorithm>

namespace score
{
class CommandStackFacade;
} // namespace score

using namespace Scenario::Command;
using namespace score::IDocument; // for ::path

namespace Scenario
{
void clearContentFromSelection(
    const QList<const IntervalModel*>& intervalsToRemove,
    const QList<const StateModel*>& statesToRemove,
    const score::DocumentContext& ctx)
{
  MacroCommandDispatcher<ClearSelection> cleaner{ctx.commandStack};

  // Create a Clear command for each.

  for (auto& state : statesToRemove)
  {
    if (state->messages().rootNode().hasChildren())
    {
      cleaner.submit(new ClearState(*state));
    }
  }

  for (auto& interval : intervalsToRemove)
  {
    cleaner.submit(new ClearInterval(*interval));
    // if a state and an interval are selected then remove event too
  }

  cleaner.commit();
}

void clearContentFromSelection(
    const Scenario::ProcessModel& scenario,
    const score::DocumentContext& stack)
{
  clearContentFromSelection(
      selectedElements(scenario.intervals), selectedElements(scenario.states),
      stack);
}

template <typename Range, typename Fun>
void erase_if(Range& r, Fun f)
{
  for (auto it = std::begin(r); it != std::end(r);)
  {
    it = f(*it) ? r.erase(it) : ++it;
  }
}

void removeSelection(
    const Scenario::ProcessModel& scenario, const score::DocumentContext& ctx)
{
  auto& stack = ctx.commandStack;
  MacroCommandDispatcher<ClearSelection> cleaner{stack};

  const QList<const StateModel*>& states
      = selectedElements(scenario.getStates());

  // Create a Clear command for each.

  for (auto& state : states)
  {
    if (state->messages().rootNode().hasChildren())
    {
      cleaner.submit(new ClearState(*state));
    }
  }

  Selection sel = scenario.selectedChildren();

  for (const auto& obj : ctx.selectionStack.currentSelection())
  {
    if (auto proc = dynamic_cast<const Process::ProcessModel*>(obj.data()))
    {
      bool remove_it = true;

      for (const auto& obj : sel)
      {
        if (auto itv = dynamic_cast<const IntervalModel*>(obj.data()))
        {
          if (itv == proc->parent())
          {
            remove_it = false;
            break;
          }
        }
      }

      if (remove_it)
        cleaner.submit(new Scenario::Command::RemoveProcessFromInterval(
            *(IntervalModel*)proc->parent(), proc->id()));
    }
  }

  score::SelectionDispatcher s{ctx.selectionStack};
  s.setAndCommit({});
  ctx.selectionStack.clear();
  // We have to remove the first / last timesyncs / events from the selection.
  erase_if(sel, [&](auto&& elt) { return elt->id_val() == startId_val; });

  if (!sel.empty())
  {
    cleaner.submit(new RemoveSelection(scenario, sel));
  }

  cleaner.commit();
}

void removeSelection(const BaseScenario&, const score::DocumentContext&)
{
  // Shall do nothing
}

void clearContentFromSelection(
    const BaseScenarioContainer& scenario, const score::DocumentContext& ctx)
{
  QList<const Scenario::IntervalModel*> itv;
  QList<const Scenario::StateModel*> states;
  if (scenario.interval().selection.get())
    itv.push_back(&scenario.interval());
  if (scenario.startState().selection.get())
    states.push_back(&scenario.startState());
  if (scenario.endState().selection.get())
    states.push_back(&scenario.endState());

  clearContentFromSelection(itv, states, ctx);
}

void clearContentFromSelection(
    const BaseScenario& scenario, const score::DocumentContext& ctx)
{
  clearContentFromSelection(
      static_cast<const BaseScenarioContainer&>(scenario), ctx);
}

void clearContentFromSelection(
    const Scenario::ScenarioInterface& scenario,
    const score::DocumentContext& stack)
{
  clearContentFromSelection(
      selectedElements(scenario.getIntervals()),
      selectedElements(scenario.getStates()), stack);
}

template <typename T>
auto make_ordered(const Scenario::ProcessModel& scenario)
{
  using comp_t = StartDateComparator<T>;
  using set_t = std::set<const T*, comp_t>;

  set_t the_set(comp_t{&scenario});

  auto cont = Scenario::ElementTraits<Scenario::ProcessModel, T>::accessor;
  for (auto& tn : selectedElements(cont(scenario)))
  {
    the_set.insert(tn);
  }
  return the_set;
}

void mergeTimeSyncs(
    const Scenario::ProcessModel& scenario, const score::CommandStackFacade& f)
{
  // We merge all the furthest timesyncs to the first one.
  auto timesyncs = make_ordered<TimeSyncModel>(scenario);
  auto states = make_ordered<StateModel>(scenario);

  if (timesyncs.size() < 2)
  {
    if (states.size() == 2)
    {
      auto it = states.begin();
      auto& first = Scenario::parentTimeSync(**it, scenario);
      auto& second = Scenario::parentTimeSync(**(++it), scenario);

      auto cmd
          = new Command::MergeTimeSyncs(scenario, second.id(), first.id());
      f.redoAndPush(cmd);
    }
  }
  else
  {
    auto it = timesyncs.begin();
    auto first_tn = (*it)->id();
    for (++it; it != timesyncs.end(); ++it)
    {
      auto cmd = new Command::MergeTimeSyncs(scenario, first_tn, (*it)->id());
      f.redoAndPush(cmd);
    }
  }
}

void mergeEvents(
    const Scenario::ProcessModel& scenario, const score::CommandStackFacade& f)
{
  // We merge all the furthest events to the first one.
  const QList<const StateModel*>& states
      = selectedElements(scenario.getStates());
  const QList<const EventModel*>& events
      = selectedElements(scenario.getEvents());

  QList<const EventModel*> sel = events;

  for (auto it = states.begin(); it != states.end(); ++it)
  {
    sel.push_back(&Scenario::parentEvent(**it, scenario));
  }

  sel = sel.toSet().toList();

  MacroCommandDispatcher<MergeEventMacro> merger{f};

  while (sel.size() > 1)
  {
    auto it = sel.begin();
    auto first_ev = *it;
    for (++it; it != sel.end();)
    {
      // Check if Events have the same TimeSync parent
      if (first_ev->timeSync() == (*it)->timeSync())
      {
        auto cmd
            = new Command::MergeEvents(scenario, first_ev->id(), (*it)->id());
        // f.redoAndPush(cmd);
        merger.submit(cmd);
        it = sel.erase(it);
      }
      else
        ++it;
    }
    sel.removeOne(first_ev);
  }
  merger.commit();
}

bool EndDateComparator::
operator()(const IntervalModel* lhs, const IntervalModel* rhs) const
{
  return (Scenario::date(*lhs, *scenario) + lhs->duration.defaultDuration())
         <= (Scenario::date(*rhs, *scenario)
             + rhs->duration.defaultDuration());
}
}
