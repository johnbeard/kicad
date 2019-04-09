/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 KiCad Developers, see AUTHORS.txt for contributors.
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
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * Unit tests for the nod (aka SIGNAL) signal-slot implementation.
 *
 * Mostly intended as documentation and tests of the nod
 * usages used in KiCad, rather than a complete test suite.
 */

#include <unit_test_utils/unit_test_utils.h>

// Code under test
#include <signal/signal.h>

#include <limits>

/**
 * Helper struct that contains some trivial and non-trivial
 * data to pass as signal arguments.
 */
struct FIS_ARGS
{
    float       m_float;
    int         m_int;
    std::string m_str;

    bool operator==( const FIS_ARGS& aOther ) const
    {
        return m_float == aOther.m_float && m_int == aOther.m_int && m_str == aOther.m_str;
    }

    bool operator!=( const FIS_ARGS& aOther ) const
    {
        return !( *this == aOther );
    }
};


/**
 * Dump FIS_ARGS to a stream: used for logging
 */
static std::ostream& operator<<( std::ostream& aOs, const FIS_ARGS& aArgs )
{
    aOs << aArgs.m_float << ", " << aArgs.m_int << ", '" << aArgs.m_str << "'";
    return aOs;
}


/**
 * A class that has some signals that can be collected
 */
class TEST_HAS_SIGNALS
{
public:
    ///> Take some args, returns an int
    SIGNAL::signal<int( float, int, std::string )> m_i_fis_sig;

    /**
     * Emit the FIS signal
     * @param  aArgs args to send in the signal
     */
    void do_emit( const FIS_ARGS& aArgs  )
    {
        // In real life some other logic might go here
        // such as a model update.

        // But all we do is update the receivers with the arguments
        m_i_fis_sig( aArgs.m_float, aArgs.m_int, aArgs.m_str );
    }

    ///> A function that can be used as an i-FIS callable
    using I_FIS_RECEIVER = std::function<int(float, int, std::string)>;

    /**
     * Make a receiver that pushes back into a given vector, and returns the
     * given argument
     * @param  aStore store to push back into
     * @param  aRet   value to return
     * @return        callable satisfying above requirements
     */
    static I_FIS_RECEIVER make_receiver( std::vector<FIS_ARGS>& aStore, int aRet )
    {
        return [&aStore, aRet]( float aF, int aI, const std::string& aS) {
            const FIS_ARGS args { aF, aI, aS };

            std::stringstream  ss;
            ss << "Receiving [ " << args << " ], returning " << aRet;
            BOOST_TEST_MESSAGE( ss.str() );

            aStore.push_back( args );
            return aRet;
        };
    }

    /**
     * Make a FIS receiver that doesn't use the arguments
     * and just returns 0
     */
    static I_FIS_RECEIVER make_null_receiver()
    {
        return []( float aF, int aI, const std::string& aS ) {
            const FIS_ARGS args { aF, aI, aS };
            const int ret = 0;

            std::stringstream  ss;
            ss << "Receiving [ " << args << " ], discarding, returning " << ret;
            BOOST_TEST_MESSAGE( ss.str() );

            return ret;
        };
    }

    /**
     * List of F-I-S args to send
     */
    const std::vector<FIS_ARGS> m_fis_cases = {
        {
            1.0f,
            42,
            "hoopy",
        },
        {
            -150.0f,
            0,
            "frood",
        },
        {
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<int>::max(),
            "",
        }
    };

    const std::vector<FIS_ARGS> m_fis_empty = {};

    ///> Collection of FIS argument receivers
    std::vector<std::vector<FIS_ARGS>> m_fis_receivers;

    void clear_all_fis_receivers()
    {
        for( auto& r : m_fis_receivers )
            r.clear();
    }
};


/**
 * Declare the test suite
 */
BOOST_FIXTURE_TEST_SUITE( Signal, TEST_HAS_SIGNALS )


/**
 * Test a simple model of connecting some signals, collecting
 * the slot calls and then disconnecting one by scoped connection
 * and the other manually.
 *
 * The return values are not checked.
 */
BOOST_AUTO_TEST_CASE( IFisSimple )
{
    m_fis_receivers.resize(2);

    // Connect the signal to the receivers
    SIGNAL::connection c1 = m_i_fis_sig.connect( make_receiver( m_fis_receivers[0], 111 ) );

    {
        // This connection lasts only for the scope
        SIGNAL::scoped_connection c2 = m_i_fis_sig.connect( make_receiver( m_fis_receivers[1], 222 ) );

        // Send all the signals together
        for( const auto& c : m_fis_cases )
        {
            do_emit( c );
        }

        // c2 will be disconnected here
    }

    // We should get the same args, in order, from each handler
    KI_TEST_CHECK_EQUAL_COLLECTIONS_WHOLE( m_fis_cases, m_fis_receivers[0] );
    KI_TEST_CHECK_EQUAL_COLLECTIONS_WHOLE( m_fis_cases, m_fis_receivers[1] );

    clear_all_fis_receivers();

    // Send all the signals again
    for( const auto& c : m_fis_cases )
    {
        do_emit( c );
    }

    // Only c1 was connected
    KI_TEST_CHECK_EQUAL_COLLECTIONS_WHOLE( m_fis_cases, m_fis_receivers[0] );
    KI_TEST_CHECK_EQUAL_COLLECTIONS_WHOLE( m_fis_empty, m_fis_receivers[1] );

    // now disconnect the non-scoped signal
    c1.disconnect();

    clear_all_fis_receivers();

    // Send all the signals again
    for( const auto& c : m_fis_cases )
    {
        do_emit( c );
    }

    // Nothing happens
    KI_TEST_CHECK_EQUAL_COLLECTIONS_WHOLE( m_fis_empty, m_fis_receivers[0] );
    KI_TEST_CHECK_EQUAL_COLLECTIONS_WHOLE( m_fis_empty, m_fis_receivers[1] );
}

BOOST_AUTO_TEST_SUITE_END()