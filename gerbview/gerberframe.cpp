/*******************/
/* gerberframe.cpp */
/*******************/

#include "fctsys.h"
#include "appl_wxstruct.h"
#include "wxstruct.h"
#include "common.h"
#include "class_drawpanel.h"

#include "gerbview.h"
#include "class_gerber_draw_item.h"
#include "pcbplot.h"
#include "bitmaps.h"
#include "gerbview_id.h"
#include "hotkeys.h"
#include "class_GERBER.h"
#include "dialog_helpers.h"
#include "class_DCodeSelectionbox.h"

#include "build_version.h"


// Config keywords
const wxString GerbviewShowPageSizeOption( wxT( "ShowPageSizeOpt" ) );
const wxString GerbviewShowDCodes( wxT( "ShowDCodesOpt" ) );

/****************************************/
/* class GERBVIEW_FRAME for GerbView*/
/****************************************/

BEGIN_EVENT_TABLE( GERBVIEW_FRAME, PCB_BASE_FRAME )
    EVT_CLOSE( GERBVIEW_FRAME::OnCloseWindow )
    EVT_SIZE( GERBVIEW_FRAME::OnSize )

    EVT_TOOL( wxID_FILE, GERBVIEW_FRAME::Files_io )
    EVT_TOOL( ID_INC_LAYER_AND_APPEND_FILE, GERBVIEW_FRAME::Files_io )
    EVT_TOOL( ID_GERBVIEW_LOAD_DRILL_FILE, GERBVIEW_FRAME::Files_io )
    EVT_TOOL( ID_GERBVIEW_LOAD_DCODE_FILE, GERBVIEW_FRAME::Files_io )
    EVT_TOOL( ID_NEW_BOARD, GERBVIEW_FRAME::Files_io )

    // Menu Files:
    EVT_MENU( wxID_FILE, GERBVIEW_FRAME::Files_io )
    EVT_MENU( ID_MENU_INC_LAYER_AND_APPEND_FILE, GERBVIEW_FRAME::Files_io )
    EVT_MENU( ID_NEW_BOARD, GERBVIEW_FRAME::Files_io )
    EVT_MENU( ID_GEN_PLOT, GERBVIEW_FRAME::ToPlotter )
    EVT_MENU( ID_GERBVIEW_EXPORT_TO_PCBNEW, GERBVIEW_FRAME::ExportDataInPcbnewFormat )

    EVT_MENU_RANGE( wxID_FILE1, wxID_FILE9, GERBVIEW_FRAME::OnFileHistory )

    EVT_MENU( ID_EXIT, GERBVIEW_FRAME::Process_Special_Functions )

    // menu Preferences
    EVT_MENU_RANGE( ID_PREFERENCES_HOTKEY_START, ID_PREFERENCES_HOTKEY_END,
                    GERBVIEW_FRAME::Process_Config )

    EVT_MENU( ID_MENU_GERBVIEW_SHOW_HIDE_LAYERS_MANAGER_DIALOG,
              GERBVIEW_FRAME::OnSelectOptionToolbar )
    EVT_MENU( ID_GERBVIEW_OPTIONS_SETUP, GERBVIEW_FRAME::InstallGerberOptionsDialog )

    EVT_MENU_RANGE( ID_LANGUAGE_CHOICE, ID_LANGUAGE_CHOICE_END, EDA_DRAW_FRAME::SetLanguage )

    // menu Postprocess
    EVT_MENU( ID_GERBVIEW_SHOW_LIST_DCODES, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_MENU( ID_GERBVIEW_POPUP_DELETE_DCODE_ITEMS, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_MENU( ID_GERBVIEW_SHOW_SOURCE, GERBVIEW_FRAME::Process_Special_Functions )

    // menu Miscellaneous
    EVT_MENU( ID_GERBVIEW_GLOBAL_DELETE, GERBVIEW_FRAME::Process_Special_Functions )

    // Menu Help
    EVT_MENU( ID_GENERAL_HELP, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( ID_KICAD_ABOUT, EDA_DRAW_FRAME::GetKicadAbout )

    EVT_TOOL( wxID_CUT, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_COPY, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_PASTE, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_UNDO, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_PRINT, GERBVIEW_FRAME::ToPrinter )
    EVT_TOOL( ID_FIND_ITEMS, GERBVIEW_FRAME::Process_Special_Functions )
    EVT_KICAD_CHOICEBOX( ID_TOOLBARH_GERBVIEW_SELECT_LAYER,
                         GERBVIEW_FRAME::Process_Special_Functions )

    EVT_SELECT_DCODE( ID_TOOLBARH_GERBER_SELECT_TOOL,
                      GERBVIEW_FRAME::Process_Special_Functions )

    // Vertical toolbar:
    EVT_TOOL( ID_NO_TOOL_SELECTED, GERBVIEW_FRAME::Process_Special_Functions )

    EVT_MENU_RANGE( ID_POPUP_GENERAL_START_RANGE, ID_POPUP_GENERAL_END_RANGE,
                    GERBVIEW_FRAME::Process_Special_Functions )

    // Pop up menu
    EVT_MENU( ID_GERBVIEW_POPUP_DELETE_DCODE_ITEMS, GERBVIEW_FRAME::Process_Special_Functions )

    // Option toolbar
    EVT_TOOL( ID_TB_OPTIONS_SHOW_POLYGONS_SKETCH,
                    GERBVIEW_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_FLASHED_ITEMS_SKETCH, GERBVIEW_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_LINES_SKETCH, GERBVIEW_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_LAYERS_MANAGER_VERTICAL_TOOLBAR,
              GERBVIEW_FRAME::OnSelectOptionToolbar )
    EVT_TOOL( ID_TB_OPTIONS_SHOW_DCODES, GERBVIEW_FRAME::OnSelectOptionToolbar )
    EVT_TOOL_RANGE( ID_TB_OPTIONS_SHOW_GBR_MODE_0, ID_TB_OPTIONS_SHOW_GBR_MODE_2,
                    GERBVIEW_FRAME::OnSelectDisplayMode )

    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_FLASHED_ITEMS_SKETCH,
                   GERBVIEW_FRAME::OnUpdateFlashedItemsDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_LINES_SKETCH, GERBVIEW_FRAME::OnUpdateLinesDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_POLYGONS_SKETCH,
                   GERBVIEW_FRAME::OnUpdatePolygonsDrawMode )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_DCODES, GERBVIEW_FRAME::OnUpdateShowDCodes )
    EVT_UPDATE_UI( ID_TB_OPTIONS_SHOW_LAYERS_MANAGER_VERTICAL_TOOLBAR,
                   GERBVIEW_FRAME::OnUpdateShowLayerManager )

    EVT_UPDATE_UI( ID_TOOLBARH_GERBER_SELECT_TOOL, GERBVIEW_FRAME::OnUpdateSelectDCode )
    EVT_UPDATE_UI( ID_TOOLBARH_GERBVIEW_SELECT_LAYER, GERBVIEW_FRAME::OnUpdateLayerSelectBox )
    EVT_UPDATE_UI_RANGE( ID_TB_OPTIONS_SHOW_GBR_MODE_0, ID_TB_OPTIONS_SHOW_GBR_MODE_2,
                         GERBVIEW_FRAME::OnUpdateDrawMode )

END_EVENT_TABLE()


GERBVIEW_FRAME::GERBVIEW_FRAME( wxWindow*       father,
                                        const wxString& title,
                                        const wxPoint&  pos,
                                        const wxSize&   size,
                                        long            style ) :
    PCB_BASE_FRAME( father, GERBER_FRAME, title, pos, size, style )
{
    m_FrameName = wxT( "GerberFrame" );
    m_show_layer_manager_tools = true;

    m_Draw_Axis = true;         // true to show X and Y axis on screen
    m_Draw_Sheet_Ref = false;   // true for reference drawings.
    m_HotkeysZoomAndGridList = s_Gerbview_Hokeys_Descr;
    m_SelLayerBox   = NULL;
    m_DCodeSelector = NULL;
    m_displayMode   = 0;

    if( DrawPanel )
        DrawPanel->m_Block_Enable = true;

    // Give an icon
#ifdef __WINDOWS__
    SetIcon( wxICON( a_icon_gerbview ) );
#else
    SetIcon( wxICON( icon_gerbview ) );
#endif

    SetScreen( ScreenPcb );

    SetBoard( new BOARD( NULL, this ) );
    GetBoard()->SetEnabledLayers( FULL_LAYERS );     // All 32 layers enabled at first.

    // Create the PCB_LAYER_WIDGET *after* SetBoard():
    wxFont font = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    int    pointSize    = font.GetPointSize();
    int    screenHeight = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );

    if( screenHeight <= 900 )
        pointSize = (pointSize * 8) / 10;

    m_LayersManager = new GERBER_LAYER_WIDGET( this, DrawPanel, pointSize );

    // LoadSettings() *after* creating m_LayersManager, because LoadSettings()
    // initialize parameters in m_LayersManager
    LoadSettings();
    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );
    GetScreen()->SetGrid( ID_POPUP_GRID_LEVEL_1000 + m_LastGridSizeId  );

    ReCreateMenuBar();
    ReCreateHToolbar();
    ReCreateOptToolbar();

    m_auimgr.SetManagedWindow( this );

    wxAuiPaneInfo horiz;
    horiz.Gripper( false );
    horiz.DockFixed( true );
    horiz.Movable( false );
    horiz.Floatable( false );
    horiz.CloseButton( false );
    horiz.CaptionVisible( false );

    wxAuiPaneInfo vert( horiz );

    vert.TopDockable( false ).BottomDockable( false );
    horiz.LeftDockable( false ).RightDockable( false );

    // LAYER_WIDGET is floatable, but initially docked at far right
    wxAuiPaneInfo lyrs;
    lyrs.CloseButton( false );
    lyrs.Caption( _( "Visibles" ) );
    lyrs.IsFloatable();

    if( m_HToolBar )
        m_auimgr.AddPane( m_HToolBar,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_HToolBar" ) ).Top().Row( 0 ) );

    if( m_VToolBar )
        m_auimgr.AddPane( m_VToolBar,
                          wxAuiPaneInfo( vert ).Name( wxT( "m_VToolBar" ) ).Right().Row( 1 ) );

    m_auimgr.AddPane( m_LayersManager,
                      lyrs.Name( wxT( "m_LayersManagerToolBar" ) ).Right().Row( 0 ) );

    if( m_OptionsToolBar )
        m_auimgr.AddPane( m_OptionsToolBar,
                          wxAuiPaneInfo( vert ).Name( wxT( "m_OptionsToolBar" ) ).Left() );

    if( DrawPanel )
        m_auimgr.AddPane( DrawPanel,
                          wxAuiPaneInfo().Name( wxT( "DrawFrame" ) ).CentrePane() );

    if( MsgPanel )
        m_auimgr.AddPane( MsgPanel,
                          wxAuiPaneInfo( horiz ).Name( wxT( "MsgPanel" ) ).Bottom() );

    ReFillLayerWidget();    // this is near end because contents establish size
    m_auimgr.Update();
}


GERBVIEW_FRAME::~GERBVIEW_FRAME()
{
    SetScreen( ScreenPcb );
    wxGetApp().SaveCurrentSetupValues( m_configSettings );
}


void GERBVIEW_FRAME::OnCloseWindow( wxCloseEvent& Event )
{
    SaveSettings();
    Destroy();
}


int GERBVIEW_FRAME::BestZoom()
{
    // gives a minimal value to zoom, if no item in list
    if( GetBoard()->m_Drawings == NULL  )
        return 16 * GetScreen()->m_ZoomScalar;

    double      x, y;
    EDA_Rect    bbox;
    BOARD_ITEM* item = GetBoard()->m_Drawings;

    bbox = ( (GERBER_DRAW_ITEM*) item )->GetBoundingBox();

    for( ; item; item = item->Next() )
    {
        GERBER_DRAW_ITEM* gerb_item = (GERBER_DRAW_ITEM*) item;
        bbox.Merge( gerb_item->GetBoundingBox() );
    }

    wxSize size = DrawPanel->GetClientSize();

    x = (double) bbox.GetWidth() / (double) size.x;
    y = (double) bbox.GetHeight() / (double) size.y;
    GetScreen()->SetScrollCenterPosition( bbox.Centre() );

    int best_zoom = wxRound( MAX( x, y ) * (double) GetScreen()->m_ZoomScalar );
    return best_zoom;
}


void GERBVIEW_FRAME::LoadSettings()
{
    wxConfig* config = wxGetApp().m_EDA_Config;

    if( config == NULL )
        return;

    PCB_BASE_FRAME::LoadSettings();

    wxGetApp().ReadCurrentSetupValues( GetConfigurationSettings() );

    long pageSize_opt;
    config->Read( GerbviewShowPageSizeOption, &pageSize_opt, 0l );
    int  imax = 0;

    for( ; g_GerberPageSizeList[imax] != NULL; imax++ )
        ;

    if( pageSize_opt < 0 || pageSize_opt >= imax )
        pageSize_opt = 0;

    GetScreen()->m_CurrentSheetDesc = g_GerberPageSizeList[pageSize_opt];

    if( pageSize_opt > 0 )
    {
        m_Draw_Sheet_Ref = true;
    }

    long tmp;
    config->Read( GerbviewShowDCodes, &tmp, 1 );
    SetElementVisibility( DCODES_VISIBLE, tmp );
}


void GERBVIEW_FRAME::SaveSettings()
{
    wxConfig* config = wxGetApp().m_EDA_Config;

    if( config == NULL )
        return;

    PCB_BASE_FRAME::SaveSettings();

    wxGetApp().SaveCurrentSetupValues( GetConfigurationSettings() );

    wxRealPoint GridSize = GetScreen()->GetGridSize();

    long        pageSize_opt = 0;

    if( m_Draw_Sheet_Ref )
    {
        for( int ii = 1; g_GerberPageSizeList[ii] != NULL; ii++ )
        {
            if( GetScreen()->m_CurrentSheetDesc == g_GerberPageSizeList[ii] )
            {
                pageSize_opt = ii;
                break;
            }
        }
    }

    config->Write( GerbviewShowPageSizeOption, pageSize_opt );
    config->Write( GerbviewShowDCodes, IsElementVisible( DCODES_VISIBLE ) );
}


void GERBVIEW_FRAME::ReFillLayerWidget()
{
    m_LayersManager->ReFill();

    wxAuiPaneInfo& lyrs = m_auimgr.GetPane( m_LayersManager );

    wxSize         bestz = m_LayersManager->GetBestSize();

    lyrs.MinSize( bestz );
    lyrs.BestSize( bestz );
    lyrs.FloatingSize( bestz );

    if( lyrs.IsDocked() )
        m_auimgr.Update();
    else
        m_LayersManager->SetSize( bestz );

    syncLayerWidget();
}


/**
 * Function IsGridVisible() , virtual
 * @return true if the grid must be shown
 */
bool GERBVIEW_FRAME::IsGridVisible()
{
    return IsElementVisible( GERBER_GRID_VISIBLE );
}


/**
 * Function SetGridVisibility() , virtual
 * It may be overloaded by derived classes
 * if you want to store/retrieve the grid visiblity in configuration.
 * @param aVisible = true if the grid must be shown
 */
void GERBVIEW_FRAME::SetGridVisibility( bool aVisible )
{
    SetElementVisibility( GERBER_GRID_VISIBLE, aVisible );
}


/**
 * Function GetGridColor() , virtual
 * @return the color of the grid
 */
int GERBVIEW_FRAME::GetGridColor()
{
    return GetBoard()->GetVisibleElementColor( GERBER_GRID_VISIBLE );
}


/**
 * Function SetGridColor() , virtual
 * @param aColor = the new color of the grid
 */
void GERBVIEW_FRAME::SetGridColor( int aColor )
{
    GetBoard()->SetVisibleElementColor( GERBER_GRID_VISIBLE, aColor );
}


/**
 * Function SetElementVisibility
 * changes the visibility of an element category
 * @param aGERBER_VISIBLE is from the enum by the same name
 * @param aNewState = The new visibility state of the element category
 * @see enum aGERBER_VISIBLE
 */
void GERBVIEW_FRAME::SetElementVisibility( int aGERBER_VISIBLE, bool aNewState )
{
    GetBoard()->SetElementVisibility( aGERBER_VISIBLE, aNewState );
    m_LayersManager->SetRenderState( aGERBER_VISIBLE, aNewState );
}


int GERBVIEW_FRAME::getNextAvailableLayer( int aLayer ) const
{
    int layer = aLayer;

    for( int i = 0;  i < NB_LAYERS;  i++ )
    {
        GERBER_IMAGE* gerber = g_GERBER_List[ layer ];

        if( gerber == NULL || gerber->m_FileName.IsEmpty() )
            return layer;

        layer++;

        if( layer >= NB_LAYERS )
            layer = 0;
    }

    return NO_AVAILABLE_LAYERS;
}


void GERBVIEW_FRAME::syncLayerWidget()
{
    m_LayersManager->SelectLayer( getActiveLayer() );
    UpdateTitleAndInfo();
}


/**
 * Function syncLayerBox
 * updates the currently "selected" layer within m_SelLayerBox
 * The currently active layer, as defined by the return value of
 * getActiveLayer().  And updates the colored icon in the toolbar.
 */
void GERBVIEW_FRAME::syncLayerBox()
{
    m_SelLayerBox->SetSelection( getActiveLayer() );
    int dcodeSelected = -1;
    GERBER_IMAGE* gerber = g_GERBER_List[getActiveLayer()];

    if( gerber )
        dcodeSelected = gerber->m_Selected_Tool;

    if( m_DCodeSelector )
    {
        m_DCodeSelector->SetDCodeSelection( dcodeSelected );
        m_DCodeSelector->Enable( gerber != NULL );
    }

    UpdateTitleAndInfo();
}


/**
 * Function SetLanguage
 * called on a language menu selection
 * Update Layer manager title and tabs texts
 */
void GERBVIEW_FRAME::SetLanguage( wxCommandEvent& event )
{
    EDA_DRAW_FRAME::SetLanguage( event );
    m_LayersManager->SetLayersManagerTabsText();
    wxAuiPaneInfo& pane_info = m_auimgr.GetPane( m_LayersManager );
    pane_info.Caption( _( "Visibles" ) );
    m_auimgr.Update();

    ReFillLayerWidget();
}


void GERBVIEW_FRAME::Liste_D_Codes()
{
    int ii, jj;
    D_CODE*           pt_D_code;
    wxString          Line;
    WinEDA_TextFrame* List;
    int               scale = 10000;
    int               curr_layer = GetScreen()->m_Active_Layer;

    List = new WinEDA_TextFrame( this, _( "List D codes" ) );

    for( int layer = 0; layer < 32; layer++ )
    {
        GERBER_IMAGE* gerber = g_GERBER_List[layer];

        if( gerber == NULL )
            continue;

        if( gerber->ReturnUsedDcodeNumber() == 0 )
            continue;

        if( layer == curr_layer )
            Line.Printf( wxT( "*** Active layer (%2.2d) ***" ), layer + 1 );
        else
            Line.Printf( wxT( "*** layer %2.2d  ***" ), layer + 1 );

        List->Append( Line );

        for( ii = 0, jj = 1; ii < TOOLS_MAX_COUNT; ii++ )
        {
            pt_D_code = gerber->GetDCODE( ii + FIRST_DCODE, false );

            if( pt_D_code == NULL )
                continue;

            if( !pt_D_code->m_InUse && !pt_D_code->m_Defined )
                continue;

            Line.Printf( wxT( "tool %2.2d:   D%2.2d  V %2.4f  H %2.4f  %s" ),
                         jj,
                         pt_D_code->m_Num_Dcode,
                         (float) pt_D_code->m_Size.y / scale,
                         (float) pt_D_code->m_Size.x / scale,
                         D_CODE::ShowApertureType( pt_D_code->m_Shape )
                         );

            if( !pt_D_code->m_Defined )
                Line += wxT( " ?" );

            if( !pt_D_code->m_InUse )
                Line += wxT( " *" );

            List->Append( Line );
            jj++;
        }
    }

    ii = List->ShowModal();
    List->Destroy();

    if( ii < 0 )
        return;
}


/*
 * Function UpdateTitleAndInfo
 * displays the short filename (if exists) of the selected layer
 *  on the caption of the main gerbview window
 * displays image name and the last layer name (found in the gerber file: LN <name> command)
 *  in the status bar
 * Note layer name can change when reading a gerber file, and the layer name is the last found.
 * So, show the layer name is not very useful, and can be seen as a debug feature.
 */
void GERBVIEW_FRAME::UpdateTitleAndInfo()
{
    GERBER_IMAGE* gerber = g_GERBER_List[ GetScreen()->m_Active_Layer ];
    wxString      text;

    // Display the gerber filename
    if( gerber == NULL )
    {
        text = wxGetApp().GetAppName() + wxT( " " ) + GetBuildVersion();
        SetTitle( text );
        SetStatusText( wxEmptyString, 0 );
        text.Printf( _( "Layer %d not in use" ), GetScreen()->m_Active_Layer + 1 );
        m_TextInfo->SetValue( text );
        ClearMsgPanel();
        return;
    }

    text = _( "File:" );
    text << wxT( " " ) << gerber->m_FileName;
    SetTitle( text );

    gerber->DisplayImageInfo();

    // Display Image Name and Layer Name (from the current gerber data):
    text.Printf( _( "Image name: \"%s\"  Layer name: \"%s\"" ),
                 GetChars( gerber->m_ImageName ),
                 GetChars( gerber->GetLayerParams().m_LayerName ) );
    SetStatusText( text, 0 );

    // Display data format like fmt in X3.4Y3.4 no LZ or fmt mm X2.3 Y3.5 no TZ in main toolbar
    text.Printf( wxT( "fmt: %s X%d.%d Y%d.%d no %cZ" ),
                 gerber->m_GerbMetric ? wxT( "mm" ) : wxT( "in" ),
                 gerber->m_FmtLen.x - gerber->m_FmtScale.x, gerber->m_FmtScale.x,
                 gerber->m_FmtLen.y - gerber->m_FmtScale.y, gerber->m_FmtScale.y,
                 gerber->m_NoTrailingZeros ? 'T' : 'L' );

    m_TextInfo->SetValue( text );
}


/* Function OnSelectDisplayMode: called to select display mode
 * (fast display, or exact mode with stacked images or with transparency
 */
void GERBVIEW_FRAME::OnSelectDisplayMode( wxCommandEvent& event )
{
    int oldMode = GetDisplayMode();

    switch( event.GetId() )
    {
    case ID_TB_OPTIONS_SHOW_GBR_MODE_0:
        SetDisplayMode( 0 );
        break;

    case ID_TB_OPTIONS_SHOW_GBR_MODE_1:
        SetDisplayMode( 1 );
        break;

    case ID_TB_OPTIONS_SHOW_GBR_MODE_2:
        SetDisplayMode( 2 );
        break;
    }

    if( GetDisplayMode() != oldMode )
        DrawPanel->Refresh();
}
