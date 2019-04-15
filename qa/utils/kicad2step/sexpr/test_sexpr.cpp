/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 KiCad Developers, see CHANGELOG.TXT for contributors.
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

/**
 * @file
 * Test suite for SEXPR::PARSER
 */

#include <unit_test_utils/unit_test_utils.h>

// Code under test
#include <sexpr/sexpr.h>

#include "sexpr_test_utils.h"

/**
 * Declare the test suite
 */
BOOST_AUTO_TEST_SUITE( Sexpr )


BOOST_AUTO_TEST_CASE( BasicConstruction )
{
    SEXPR::SEXPR_INTEGER s_int{ 1 };
    // not sure why cast is needed, but boost doesn't like it without
    BOOST_CHECK_PREDICATE( KI_TEST::SexprIsIntegerWithValue, ( (SEXPR::SEXPR&) s_int )( 1 ) );

    SEXPR::SEXPR_DOUBLE s_double{ 3.14 };
    BOOST_CHECK_PREDICATE( KI_TEST::SexprIsDoubleWithValue, ( (SEXPR::SEXPR&) s_double )( 3.14 ) );

    SEXPR::SEXPR_STRING s_string{ "string" };
    BOOST_CHECK_PREDICATE(
            KI_TEST::SexprIsStringWithValue, ( (SEXPR::SEXPR&) s_string )( "string" ) );

    SEXPR::SEXPR_STRING s_symbol{ "symbol" };
    BOOST_CHECK_PREDICATE(
            KI_TEST::SexprIsStringWithValue, ( (SEXPR::SEXPR&) s_symbol )( "symbol" ) );
}

BOOST_AUTO_TEST_SUITE_END()