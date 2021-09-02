/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
#include "udp-reliable-helper.h"
#include "udp-reliable-echo-server.h"
#include "udp-reliable-echo-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

UdpReliableServerHelper::UdpReliableServerHelper (uint16_t port)
{
  m_factory.SetTypeId (UdpReliableEchoServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void 
UdpReliableServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpReliableServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpReliableServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpReliableServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpReliableServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpReliableEchoServer> ();
  node->AddApplication (app);

  return app;
}

UdpReliableClientHelper::UdpReliableClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpReliableEchoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

UdpReliableClientHelper::UdpReliableClientHelper (Address address)
{
  m_factory.SetTypeId (UdpReliableEchoClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
UdpReliableClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
UdpReliableClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<UdpReliableEchoClient>()->SetFill (fill);
}

void
UdpReliableClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<UdpReliableEchoClient>()->SetFill (fill, dataLength);
}

void
UdpReliableClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<UdpReliableEchoClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
UdpReliableClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpReliableClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpReliableClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpReliableClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpReliableEchoClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
