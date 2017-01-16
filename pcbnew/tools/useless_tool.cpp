
#include "useless_tool.h"

#include <wxPcbStruct.h>
#include <class_draw_panel_gal.h>
#include <view/view_controls.h>
#include <view/view.h>
#include <tool/tool_manager.h>

#include <pcbnew_id.h>

#include <class_board_item.h>
#include <class_drawsegment.h>
#include <board_commit.h>

#include "common_actions.h"
#include "selection_tool.h"


USELESS_TOOL::USELESS_TOOL() :
        PCB_TOOL( "pcbnew.UselessTool" ),
        m_menu( *this )
{
}


USELESS_TOOL::~USELESS_TOOL()
{}


void USELESS_TOOL::Reset( RESET_REASON aReason )
{
}


bool USELESS_TOOL::Init()
{
    auto& menu = m_menu.GetMenu();

    // add our own tool's action
    menu.AddItem( COMMON_ACTIONS::uselessFixedCircle);
    // add the PCB_EDITOR_CONTROL's zone unfill all action
    menu.AddItem( COMMON_ACTIONS::zoneUnfillAll);

    // Add standard zoom and grid tool actions
    m_menu.AddStandardSubMenus( *getEditFrame<PCB_BASE_FRAME>() );

    return true;
}

void USELESS_TOOL::moveLeftInt()
{
    // we will call actions on the selection tool to get the current
    // selection. The selection tools will handle item deisambiguation
    SELECTION_TOOL* selectionTool = m_toolMgr->GetTool<SELECTION_TOOL>();
    assert( selectionTool );

    // call the actions
    m_toolMgr->RunAction( COMMON_ACTIONS::selectionClear, true );
    m_toolMgr->RunAction( COMMON_ACTIONS::selectionCursor, true );
    selectionTool->SanitizeSelection();

    const SELECTION& selection = selectionTool->GetSelection();

    // nothing selected, return to event loop
    if( selection.Empty() )
        return;

    // iterate BOARD_ITEM* container, moving each item
    for( auto item : selection )
    {
        item->Move( wxPoint(-5 * IU_PER_MM, 0) );
    }
}

int USELESS_TOOL::moveLeft( const TOOL_EVENT& aEvent )
{
    auto& frame = *getEditFrame<PCB_EDIT_FRAME>();

    // set tool hint and cursor (actually looks like a crosshair)
    frame.SetToolID( ID_PCB_SHOW_1_RATSNEST_BUTT,
            wxCURSOR_PENCIL, _( "Select item to move left" ) );

    getViewControls()->ShowCursor( true );

    Activate();

    // handle tool events for as long as the tool is active
    while( OPT_TOOL_EVENT evt = Wait() )
    {
        if( evt->IsCancel() || evt->IsActivate() )
        {
            // end of interactive tool
            break;
        }
        else if( evt->IsClick( BUT_RIGHT ) )
        {
            m_menu.ShowContextMenu();
        }
        else if( evt->IsClick( BUT_LEFT ) )
        {
            // invoke the main action logic
            moveLeftInt();

            // keep showing the edit cursor
            getViewControls()->ShowCursor( true );
        }
        else if (evt->GetCommandStr() )
        {
            //wxLogDebug( "Pass event: ");
            //m_toolMgr->PassEvent();
            if ( evt->GetCommandStr().value() == "pcbnew.UselessTool.FixedCircle")
            {
                fixedCircle( aEvent );

            }
        }
    }

    // reset the PCB frame to how it was we got it
    frame.SetToolID( ID_NO_TOOL_SELECTED, wxCURSOR_DEFAULT, wxEmptyString );
    getViewControls()->ShowCursor( false );

    // exit action
    return 0;
}


int USELESS_TOOL::fixedCircle( const TOOL_EVENT& aEvent )
{
    auto& frame = *getEditFrame<PCB_EDIT_FRAME>();

    // new circle to add (ideally use a smart pointer)
    DRAWSEGMENT* circle = new DRAWSEGMENT;

    // Set the circle attributes
    circle->SetShape( S_CIRCLE );
    circle->SetWidth( 5 * IU_PER_MM );
    circle->SetStart( wxPoint( 50 * IU_PER_MM, 50 * IU_PER_MM ) );
    circle->SetEnd( wxPoint( 80 * IU_PER_MM, 80 * IU_PER_MM ) );
    circle->SetLayer(  LAYER_ID::F_SilkS );

    // commit the circle to the BOARD
    BOARD_COMMIT commit( &frame );
    commit.Add( circle );
    commit.Push( _( "Draw a circle" ) );

    return 0;
}


void USELESS_TOOL::SetTransitions()
{
    Go( &USELESS_TOOL::fixedCircle, COMMON_ACTIONS::uselessFixedCircle.MakeEvent() );
    Go( &USELESS_TOOL::moveLeft,  COMMON_ACTIONS::uselessMoveItemLeft.MakeEvent() );
}
