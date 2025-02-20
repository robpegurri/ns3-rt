/*
 * Copyright (c) 2005,2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "propagation-delay-model.h"

#include "ns3/double.h"
#include "ns3/mobility-model.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(PropagationDelayModel);

TypeId
PropagationDelayModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::PropagationDelayModel").SetParent<Object>().SetGroupName("Propagation");
    return tid;
}

PropagationDelayModel::~PropagationDelayModel()
{
}

int64_t
PropagationDelayModel::AssignStreams(int64_t stream)
{
    return DoAssignStreams(stream);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED(RandomPropagationDelayModel);

TypeId
RandomPropagationDelayModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::RandomPropagationDelayModel")
            .SetParent<PropagationDelayModel>()
            .SetGroupName("Propagation")
            .AddConstructor<RandomPropagationDelayModel>()
            .AddAttribute("Variable",
                          "The random variable which generates random delays (s).",
                          StringValue("ns3::UniformRandomVariable"),
                          MakePointerAccessor(&RandomPropagationDelayModel::m_variable),
                          MakePointerChecker<RandomVariableStream>());
    return tid;
}

RandomPropagationDelayModel::RandomPropagationDelayModel()
{
}

RandomPropagationDelayModel::~RandomPropagationDelayModel()
{
}

Time
RandomPropagationDelayModel::GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
    return Seconds(m_variable->GetValue());
}

int64_t
RandomPropagationDelayModel::DoAssignStreams(int64_t stream)
{
    m_variable->SetStream(stream);
    return 1;
}

NS_OBJECT_ENSURE_REGISTERED(ConstantSpeedPropagationDelayModel);

TypeId
ConstantSpeedPropagationDelayModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ConstantSpeedPropagationDelayModel")
            .SetParent<PropagationDelayModel>()
            .SetGroupName("Propagation")
            .AddConstructor<ConstantSpeedPropagationDelayModel>()
            .AddAttribute("Speed",
                          "The propagation speed (m/s) in the propagation medium being considered. "
                          "The default value is the propagation speed of light in the vacuum.",
                          DoubleValue(299792458),
                          MakeDoubleAccessor(&ConstantSpeedPropagationDelayModel::m_speed),
                          MakeDoubleChecker<double>());
    return tid;
}

ConstantSpeedPropagationDelayModel::ConstantSpeedPropagationDelayModel()
{
    SionnaHelper& sionnaHelper = SionnaHelper::GetInstance();
    if (sionnaHelper.GetSionna())
    {
        SetSionnaUp();
    }
}

Time
ConstantSpeedPropagationDelayModel::GetDelay(Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
    // 1 - Get xyz Coordinates, Velocities and Heading Angle for a and b
    Vector a_position = a->GetPosition();
    Vector b_position = b->GetPosition();
    Vector a_velocity = a->GetVelocity();
    Vector b_velocity = b->GetVelocity();

    double distance = a->GetDistanceFrom(b);

    double seconds = distance / m_speed;
    double milliseconds = seconds * 1000;

    double ns3_delay = seconds;
    double ns3_delay_ms = milliseconds;
    double sionna_delay, sionna_delay_ms;

    if (m_sionna)
    {
        // 2 - Retreive the NodeID associated to the Ptr<MobilityModel>
        Ptr<Node> nodeA = a->GetObject<Node>();
        NS_ABORT_MSG_IF(!nodeA, "Error: Ptr<MobilityModel> a (usually TX) not linked to a Node. This is needed for Sionna to track the object location!");
        std::string a_id = "obj" + std::to_string(nodeA->GetId() + 1);
        Ptr<Node> nodeB = b->GetObject<Node>();
        NS_ABORT_MSG_IF(!nodeB, "Error: Ptr<MobilityModel> b (usually RX) not linked to a Node. This is needed for Sionna to track the object location!");
        std::string b_id = "obj" + std::to_string(nodeB->GetId() + 1);

        // 3 - Location Update to Sionna
        updateLocationInSionna(a_id, a_position, a_velocity);
        updateLocationInSionna(b_id, b_position, b_velocity);

        sionna_delay = getPropagationDelayFromSionna(a_position, b_position);
        sionna_delay_ms = sionna_delay * 1000;

        if (sionna_delay != 0)
        {
            if (sionna_verbose)
            {
                printf("ns3_ms: %f, sionna_ms: %f, ", ns3_delay_ms, sionna_delay_ms);
            }

            std::string log_delays = std::to_string(ns3_delay_ms) + "," + std::to_string(sionna_delay_ms);
            logProgress(0, log_delays);
        }
    }

    seconds = m_sionna ? sionna_delay : ns3_delay;
    return Seconds(seconds);
}

void
ConstantSpeedPropagationDelayModel::SetSpeed(double speed)
{
    m_speed = speed;
}

double
ConstantSpeedPropagationDelayModel::GetSpeed() const
{
    return m_speed;
}

int64_t
ConstantSpeedPropagationDelayModel::DoAssignStreams(int64_t stream)
{
    return 0;
}

} // namespace ns3