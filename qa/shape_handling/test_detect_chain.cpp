/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>

#include <geometry/shape_poly_set.h>
#include <geometry/shape_line_chain.h>

// #include <qa/data/fixtures_geometry.h>

namespace SHAPE_HANDLING_DATA
{

std::vector< std::pair<VECTOR2I, VECTOR2I> > single_seg = {
    { { 0, 0 }, { 10, 10 } }
};

}

namespace SHD = SHAPE_HANDLING_DATA;

struct DetectChainFixture
{

};

/**
 * Declares the Boost test suite fixture.
 */
BOOST_FIXTURE_TEST_SUITE( ShapeHandling, DetectChainFixture )

/**
 * Checks whether the iteration on the segments of a common polygon is correct.
 */
BOOST_AUTO_TEST_CASE( DetectChain )
{
    BOOST_CHECK_EQUAL(SHD::single_seg[0].first, SHD::single_seg[0].second);
}

BOOST_AUTO_TEST_SUITE_END()