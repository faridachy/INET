//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.radio.ideal;

import inet.physicallayer.radio.common.RadioChannelBase;

//
// Radio channel module for ~IdealRadio.
//
// ~IdealRadioChannel should have exactly one instance in every network model that
// contains wireless nodes with ~IdealRadio. This module gets informed about the
// location and movement of nodes, and determines which nodes are within
// communication distance.
//
simple IdealRadioChannel extends RadioChannelBase
{
    parameters:
        @class(IdealRadioChannel);
}
