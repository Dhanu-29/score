#include "ReplaceConstraintContent.hpp"

#include "Document/Constraint/ConstraintModel.hpp"
#include "Document/Constraint/Rack/RackModel.hpp"
#include "Document/Constraint/Rack/Slot/SlotModel.hpp"
#include "ProcessInterface/Process.hpp"
#include "ProcessInterface/LayerModel.hpp"
#include <iscore/tools/SettableIdentifierGeneration.hpp>
#include <QJsonDocument>

using namespace iscore;
using namespace Scenario::Command;

ReplaceConstraintContent::ReplaceConstraintContent(
        QJsonObject&& sourceConstraint,
        Path<ConstraintModel>&& targetConstraint,
        ExpandMode mode) :
    SerializableCommand {factoryName(),
                         commandName(),
                         description()},
    m_source{sourceConstraint},
    m_target{targetConstraint},
    m_mode{mode}
{
    auto& trg_constraint = m_target.find();
    ConstraintModel src_constraint{
            Deserializer<JSONObject>{m_source},
            &trg_constraint}; // Temporary parent

    // For all rackes in source, generate new id's
    const auto& target_rackes = trg_constraint.racks;
    QVector<Id<RackModel>> target_rackes_ids;
    std::transform(target_rackes.begin(), target_rackes.end(),
                   std::back_inserter(target_rackes_ids),
                   [] (const auto& rack) { return rack.id(); });

    for(const auto& rack : src_constraint.racks)
    {
        auto newId = iscore::id_generator::getStrongId(target_rackes_ids);
        m_rackIds.insert(rack.id(), newId);
        target_rackes_ids.append(newId);
    }

    // Same for processes
    const auto& target_processes = trg_constraint.processes;
    QVector<Id<Process>> target_processes_ids;
    std::transform(target_processes.begin(), target_processes.end(),
                   std::back_inserter(target_processes_ids),
                   [] (const auto& proc) { return proc.id(); });

    for(const auto& proc : src_constraint.processes)
    {
        auto newId = iscore::id_generator::getStrongId(target_processes_ids);
        m_processIds.insert(proc.id(), newId);
        target_processes_ids.append(newId);
    }
}

void ReplaceConstraintContent::undo() const
{
    // We just have to remove what we added
    auto& trg_constraint = m_target.find();

    for(const auto& proc_id : m_processIds)
    {
        trg_constraint.processes.remove(proc_id);
    }

    for(const auto& rack_id : m_rackIds)
    {
        trg_constraint.racks.remove(rack_id);
    }
}


void ReplaceConstraintContent::redo() const
{
    auto& trg_constraint = m_target.find();
    ConstraintModel src_constraint{
            Deserializer<JSONObject>{m_source},
            &trg_constraint}; // Temporary parent

    std::map<const Process*, Process*> processPairs;

    // Clone the processes
    const auto& src_procs = src_constraint.processes;
    for(const auto& sourceproc : src_procs)
    {
        auto newproc = sourceproc.clone(m_processIds[sourceproc.id()], &trg_constraint);

        processPairs.insert(std::make_pair(&sourceproc, newproc));
        trg_constraint.processes.add(newproc);

        // Resize the processes according to the new constraint.
        if(m_mode == ExpandMode::Scale)
        {
            newproc->setDurationAndScale(trg_constraint.duration.defaultDuration());
        }
        else if (m_mode == ExpandMode::Grow)
        {
            newproc->setDurationAndGrow(trg_constraint.duration.defaultDuration());
        }
    }

    // Clone the rackes
    const auto& src_racks = src_constraint.racks;
    for(const auto& sourcerack: src_racks)
    {
        // A note about what happens here :
        // Since we want to duplicate our process view models using
        // the target constraint's cloned shared processes (they might setup some specific data),
        // we maintain a pair mapping each original process to their cloned counterpart.
        // We can then use the correct cloned process to clone the process view model.
        auto newrack = new RackModel{
                sourcerack,
                m_rackIds[sourcerack.id()],
                [&] (const SlotModel& source, SlotModel& target)
                {
                    for(const auto& lm : source.layers)
                    {
                        // We can safely reuse the same id since it's in a different slot.
                        auto proc = processPairs[&lm.processModel()];
                        // TODO harmonize the order of parameters (source first, then new id)
                        target.layers.add(proc->cloneLayer(lm.id(), lm, &target));
                    }
                },
                &trg_constraint};
        trg_constraint.racks.add(newrack);
    }
}

void ReplaceConstraintContent::serializeImpl(QDataStream& s) const
{
    s << m_source << m_target << m_rackIds << m_processIds << (int) m_mode;
}

void ReplaceConstraintContent::deserializeImpl(QDataStream& s)
{
    int mode;
    s >> m_source >> m_target >> m_rackIds >> m_processIds >> mode;
    m_mode = static_cast<ExpandMode>(mode);
}
