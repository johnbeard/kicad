/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file gerbview_frame.cpp
 */

#include <fctsys.h>
#include <kiface_i.h>
#include <pgm_base.h>
#include <eda_base_frame.h>
#include <class_drawpanel.h>
#include <build_version.h>
#include <trigo.h>
#include <base_units.h>
#include <gbr_layer_box_selector.h>
#include <msgpanel.h>
#include <bitmaps.h>
#include <wildcards_and_files_ext.h>
#include <eda_dockart.h>

#include <gerbview.h>
#include <gerbview_frame.h>
#include <gerbview_id.h>
#include <hotkeys.h>
#include <gerber_file_image.h>
#include <gerber_file_image_list.h>
#include <dialog_helpers.h>
#include <DCodeSelectionbox.h>
#include <gerbview_layer_widget.h>

#include <gerbview_draw_panel_gal.h>
#include <gal/graphics_abstraction_layer.h>
#include <tool/tool_manager.h>
#include <tool/tool_dispatcher.h>
#include <tools/gerbview_actions.h>
#include <view/view.h>
#include <gerbview_painter.h>


// Config keywords
static const wxString   cfgShowPageSizeOption( wxT( "PageSizeOpt" ) );
static const wxString   cfgShowDCodes( wxT( "ShowDCodesOpt" ) );
static const wxString   cfgShowNegativeObjects( wxT( "ShowNegativeObjectsOpt" ) );
static const wxString   cfgShowBorderAndTitleBlock( wxT( "ShowBorderAndTitleBlock" ) );

// Colors for layers and items
COLORS_DESIGN_SETTINGS g_ColorsSettings( FRAME_GERBER );

GERBVIEW_FRAME::GERBVIEW_FRAME( KIWAY* aKiway, wxWindow* aParent ):
    EDA_DRAW_FRAME( aKiway, aParent, FRAME_GERBER, wxT( "GerbView" ),
        wxDefaultPosition, wxDefaultSize, KICAD_DEFAULT_DRAWFRAME_STYLE, GERBVIEW_FRAME_NAME ),
    m_zipFileHistory( DEFAULT_FILE_HISTORY_SIZE, ID_GERBVIEW_ZIP_FILE1 ),
    m_drillFileHistory( DEFAULT_FILE_HISTORY_SIZE, ID_GERBVIEW_DRILL_FILE1 ),
    m_jobFileHistory( DEFAULT_FILE_HISTORY_SIZE, ID_GERBVIEW_JOB_FILE1 )
{
    m_colorsSettings = &g_ColorsSettings;
    m_gerberLayout = NULL;
    m_zoomLevelCoeff = ZOOM_FACTOR( 110 );   // Adjusted to roughly displays zoom level = 1
                                             // when the screen shows a 1:1 image
                                             // obviously depends on the monitor,
                                             // but this is an acceptable value

    // Be sure a page info is set. this default value will be overwritten later.
    PAGE_INFO pageInfo( wxT( "GERBER" ) );
    SetPageSettings( pageInfo );

    m_show_layer_manager_tools = true;

    m_showAxis = true;                      // true to show X and Y axis on screen
    m_showBorderAndTitleBlock = false;      // true for reference drawings.
    m_hotkeysDescrList = GerbviewHokeysDescr;
    m_SelLayerBox   = NULL;
    m_DCodeSelector = NULL;
    m_SelComponentBox = nullptr;
    m_SelNetnameBox = nullptr;
    m_SelAperAttributesBox = nullptr;
    m_displayMode   = 0;

    int fileHistorySize;
    Pgm().CommonSettings()->Read( FILE_HISTORY_SIZE_KEY, &fileHistorySize, DEFAULT_FILE_HISTORY_SIZE );
    m_drillFileHistory.SetMaxFiles( fileHistorySize );
    m_zipFileHistory.SetMaxFiles( fileHistorySize );
    m_jobFileHistory.SetMaxFiles( fileHistorySize );

    EDA_DRAW_PANEL_GAL* galCanvas = new GERBVIEW_DRAW_PANEL_GAL( this, -1, wxPoint( 0, 0 ),
                                            m_FrameSize,
                                            GetGalDisplayOptions(),
                                            EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE );

    SetGalCanvas( galCanvas );

    // GerbView requires draw priority for rendering negative objects
    galCanvas->GetView()->UseDrawPriority( true );

    if( m_canvas )
        m_canvas->SetEnableBlockCommands( true );

    // Give an icon
    wxIcon icon;
    icon.CopyFromBitmap( KiBitmap( icon_gerbview_xpm ) );
    SetIcon( icon );

    SetLayout( new GBR_LAYOUT() );

    SetVisibleLayers( -1 );         // All draw layers visible.

    SetScreen( new GBR_SCREEN( GetPageSettings().GetSizeIU() ) );

    // Create the PCB_LAYER_WIDGET *after* SetLayout():
    m_LayersManager = new GERBER_LAYER_WIDGET( this, m_canvas );

    // LoadSettings() *after* creating m_LayersManager, because LoadSettings()
    // initialize parameters in m_LayersManager
    LoadSettings( config() );

    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    if( m_LastGridSizeId < 0 )
        m_LastGridSizeId = 0;

    if( m_LastGridSizeId > ID_POPUP_GRID_LEVEL_0_0_1MM-ID_POPUP_GRID_LEVEL_1000 )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_0_0_1MM-ID_POPUP_GRID_LEVEL_1000;

    GetScreen()->SetGrid( ID_POPUP_GRID_LEVEL_1000 + m_LastGridSizeId  );

    ReCreateMenuBar();
    ReCreateHToolbar();
    ReCreateOptToolbar();
    ReCreateAuxiliaryToolbar();

    m_auimgr.SetManagedWindow( this );
    m_auimgr.SetArtProvider( new EDA_DOCKART( this ) );

    m_auimgr.AddPane( m_mainToolBar, EDA_PANE().HToolbar().Name( "MainToolbar" ).Top().Layer(6) );
    m_auimgr.AddPane( m_auxiliaryToolBar, EDA_PANE().HToolbar().Name( "AuxToolbar" ).Top().Layer(4) );
    m_auimgr.AddPane( m_messagePanel, EDA_PANE().Messages().Name( "MsgPanel" ).Bottom().Layer(6) );

    m_auimgr.AddPane( m_optionsToolBar, EDA_PANE().VToolbar().Name( "OptToolbar" ).Left().Layer(3) );
    m_auimgr.AddPane( m_LayersManager, EDA_PANE().Palette().Name( "LayersManager" ).Right().Layer(3)
                      .Caption( _( "Layers Manager" ) ).PaneBorder( false )
                      .MinSize( 80, -1 ).BestSize( m_LayersManager->GetBestSize() ) );

    m_auimgr.AddPane( m_canvas, EDA_PANE().Canvas().Name( "DrawFrame" ).Center() );
    m_auimgr.AddPane( GetGalCanvas(), EDA_PANE().Canvas().Name( "DrawFrameGal" ).Center().Hide() );

    ReFillLayerWidget();                // this is near end because contents establish size
    m_auimgr.Update();

    setupTools();

    SetActiveLayer( 0, true );
    Zoom_Automatique( false );           // Gives a default zoom value

    EDA_DRAW_PANEL_GAL::GAL_TYPE canvasType = LoadCanvasTypeSetting();

    // Nudge user to switch to OpenGL if they are on legacy or Cairo
    if( m_firstRunDialogSetting < 1 )
    {
        if( canvasType != EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL )
        {
            wxString msg = _( "KiCad can use your graphics card to give you a smoother "
                              "and faster experience. This option is turned off by "
                              "default since it is not compatible with all computers.\n\n"
                              "Would you like to try enabling graphics acceleration?\n\n"
                              "If you'd like to choose later, select Modern Toolset "
                              "(Accelerated) in the Preferences menu." );

            wxMessageDialog dlg( this, msg, _( "Enable Graphics Acceleration" ), wxYES_NO );

            dlg.SetYesNoLabels( _( "&Enable Acceleration" ), _( "&No Thanks" ) );

            if( dlg.ShowModal() == wxID_YES )
            {
                // Save Cairo as default in case OpenGL crashes
                saveCanvasTypeSetting( EDA_DRAW_PANEL_GAL::GAL_TYPE_CAIRO );

                // Switch to OpenGL, which will save the new setting if successful
                wxCommandEvent evt( wxEVT_MENU, ID_MENU_CANVAS_OPENGL );
                GetEventHandler()->ProcessEvent( evt );

                // Switch back to Cairo if OpenGL is not supported
                if( GetGalCanvas()->GetBackend() == EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE )
                {
                    wxCommandEvent cairoEvt( wxEVT_MENU, ID_MENU_CANVAS_CAIRO );
                    GetEventHandler()->ProcessEvent( cairoEvt );
                }
            }
            else if( canvasType == EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE )
            {
                // If they were on legacy, switch them to Cairo
                wxCommandEvent evt( wxEVT_MENU, ID_MENU_CANVAS_CAIRO );
                GetEventHandler()->ProcessEvent( evt );
            }
        }

        m_firstRunDialogSetting = 1;
        SaveSettings( config() );
    }
    else if( canvasType != EDA_DRAW_PANEL_GAL::GAL_TYPE_NONE )
    {
        if( GetGalCanvas()->SwitchBackend( canvasType ) )
            UseGalCanvas( true );

        wxUpdateUIEvent e;
        OnUpdateSwitchCanvas( e );
    }
    else
    {
        m_colorsSettings->SetLegacyMode( true );
        m_canvas->Refresh();
    }

    // Enable the axes to match legacy draw style
    auto& galOptions = GetGalDisplayOptions();
    galOptions.m_axesEnabled = true;
    galOptions.NotifyChanged();

    m_LayersManager->ReFill();
    m_LayersManager->ReFillRender();    // Update colors in Render after the config is read
}


GERBVIEW_FRAME::~GERBVIEW_FRAME()
{
    GetGalCanvas()->GetView()->Clear();

    GetGerberLayout()->GetImagesList()->DeleteAllImages();
    delete m_gerberLayout;
}


void GERBVIEW_FRAME::OnCloseWindow( wxCloseEvent& Event )
{
    GetGalCanvas()->GetView()->Clear();
    GetGalCanvas()->StopDrawing();

    if( m_toolManager )
        m_toolManager->DeactivateTool();

    if( IsGalCanvasActive() )
    {
        // Be sure any OpenGL event cannot be fired after frame deletion:
        GetGalCanvas()->SetEvtHandlerEnabled( false );
    }

    Destroy();
}


bool GERBVIEW_FRAME::OpenProjectFiles( const std::vector<wxString>& aFileSet, int aCtl )
{
    // Ensure the frame is shown when opening the file(s), to avoid issues (crash) on GAL
    // when trying to change the view if it is not fully initialized.
    // It happens when starting Gerbview with a gerber job file to load
    if( !IsShown() )
        Show();

    // The current project path is also a valid command parameter.  Check if a single path
    // rather than a file name was passed to GerbView and use it as the initial MRU path.
    if( aFileSet.size() > 0 )
    {
        wxString path = aFileSet[0];

        // For some reason wxApp appears to leave the trailing double quote on quoted
        // parameters which are required for paths with spaces.  Maybe this should be
        // pushed back into PGM_SINGLE_TOP::OnPgmInit() but that may cause other issues.
        // We can't buy a break!
        if( path.Last() ==  wxChar( '\"' ) )
            path.RemoveLast();

        if( !wxFileExists( path ) && wxDirExists( path ) )
        {
            wxLogDebug( wxT( "MRU path: %s." ), GetChars( path ) );
            m_mruPath = path;
            return true;
        }

        const unsigned limit = std::min( unsigned( aFileSet.size() ),
                                         unsigned( GERBER_DRAWLAYERS_COUNT ) );

        int layer = 0;

        for( unsigned i = 0; i < limit; ++i, ++layer )
        {
            SetActiveLayer( layer );

            // Try to guess the type of file by its ext
            // if it is .drl (Kicad files), it is a drill file
            wxFileName fn( aFileSet[i] );
            wxString ext = fn.GetExt();

            if( ext == DrillFileExtension )     // In Excellon format
                LoadExcellonFiles( aFileSet[i] );
            else if( ext == GerberJobFileExtension )
                LoadGerberJobFile( aFileSet[i] );
            else
                LoadGerberFiles( aFileSet[i] );
        }
    }

    Zoom_Automatique( true );        // Zoom fit in frame

    return true;
}


double GERBVIEW_FRAME::BestZoom()
{
    EDA_RECT bbox = GetGerberLayout()->ComputeBoundingBox();

    // Reserve a margin around the bounding box, for a better display.
    double margin_scale_factor = 1.05;

    // If there is not item loaded, use the current page size
    if( bbox.GetWidth() == 0 || bbox.GetHeight() == 0 )
    {
        bbox.SetSize( GetPageSizeIU() );
        bbox.SetOrigin( 0, 0 );
    }

    double  sizeX = (double) bbox.GetWidth();
    double  sizeY = (double) bbox.GetHeight();
    wxPoint centre = bbox.Centre();

    return bestZoom( sizeX, sizeY, margin_scale_factor, centre );
}


void GERBVIEW_FRAME::LoadSettings( wxConfigBase* aCfg )
{
    EDA_DRAW_FRAME::LoadSettings( aCfg );

    // was: wxGetApp().ReadCurrentSetupValues( GetConfigurationSettings() );
    wxConfigLoadSetups( aCfg, GetConfigurationSettings() );

    aCfg->Read( cfgShowBorderAndTitleBlock, &m_showBorderAndTitleBlock, false );

    PAGE_INFO pageInfo( wxT( "GERBER" ) );
    wxString pageType;

    aCfg->Read( cfgShowPageSizeOption, &pageType, wxT( "GERBER" ) );
    pageInfo.SetType( pageType );
    SetPageSettings( pageInfo );

    bool tmp;
    aCfg->Read( cfgShowDCodes, &tmp, true );
    SetElementVisibility( LAYER_DCODES, tmp );
    aCfg->Read( cfgShowNegativeObjects, &tmp, false );
    SetElementVisibility( LAYER_NEGATIVE_OBJECTS, tmp );

    // because we have more than one file history, we must read this one
    // using a specific path
    aCfg->SetPath( wxT( "drl_files" ) );
    m_drillFileHistory.Load( *aCfg );
    aCfg->SetPath( wxT( ".." ) );

    // because we have more than one file history, we must read this one
    // using a specific path
    aCfg->SetPath( wxT( "zip_files" ) );
    m_zipFileHistory.Load( *aCfg );
    aCfg->SetPath( wxT( ".." ) );

    // because we have more than one file history, we must read this one
    // using a specific path
    aCfg->SetPath( "job_files" );
    m_jobFileHistory.Load( *aCfg );
    aCfg->SetPath( wxT( ".." ) );
}


void GERBVIEW_FRAME::SaveSettings( wxConfigBase* aCfg )
{
    EDA_DRAW_FRAME::SaveSettings( aCfg );

    // was: wxGetApp().SaveCurrentSetupValues( GetConfigurationSettings() );
    wxConfigSaveSetups( aCfg, GetConfigurationSettings() );

    aCfg->Write( cfgShowPageSizeOption, GetPageSettings().GetType() );
    aCfg->Write( cfgShowBorderAndTitleBlock, m_showBorderAndTitleBlock );
    aCfg->Write( cfgShowDCodes, IsElementVisible( LAYER_DCODES ) );
    aCfg->Write( cfgShowNegativeObjects, IsElementVisible( LAYER_NEGATIVE_OBJECTS ) );

    // Save the drill file history list.
    // Because we have  more than one file history, we must save this one
    // in a specific path
    aCfg->SetPath( wxT( "drl_files" ) );
    m_drillFileHistory.Save( *aCfg );
    aCfg->SetPath( wxT( ".." ) );

    // Save the zip file history list.
    aCfg->SetPath( wxT( "zip_files" ) );
    m_zipFileHistory.Save( *aCfg );
    aCfg->SetPath( wxT( ".." ) );

    // Save the job file history list.
    aCfg->SetPath( "job_files" );
    m_jobFileHistory.Save( *aCfg );
    aCfg->SetPath( ".." );
}


void GERBVIEW_FRAME::ReFillLayerWidget()
{
    m_LayersManager->ReFill();
    m_SelLayerBox->Resync();
    ReCreateAuxiliaryToolbar();

    wxAuiPaneInfo&  lyrs = m_auimgr.GetPane( m_LayersManager );
    wxSize          bestz = m_LayersManager->GetBestSize();
    bestz.x += 5;   // gives a little margin

    lyrs.MinSize( bestz );
    lyrs.BestSize( bestz );
    lyrs.FloatingSize( bestz );

    if( lyrs.IsDocked() )
        m_auimgr.Update();
    else
        m_LayersManager->SetSize( bestz );

    syncLayerWidget();
}


void GERBVIEW_FRAME::SetElementVisibility( int aLayerID, bool aNewState )
{
    bool dcodes_changed = false;

    switch( aLayerID )
    {
    case LAYER_DCODES:
        dcodes_changed = m_DisplayOptions.m_DisplayDCodes != aNewState;
        m_DisplayOptions.m_DisplayDCodes = aNewState;
        break;

    case LAYER_NEGATIVE_OBJECTS:
    {
        m_DisplayOptions.m_DisplayNegativeObjects = aNewState;

        auto view = GetGalCanvas()->GetView();

        view->UpdateAllItemsConditionally( KIGFX::REPAINT, []( KIGFX::VIEW_ITEM* aItem )
        {
            auto item = static_cast<GERBER_DRAW_ITEM*>( aItem );

            // GetLayerPolarity() returns true for negative items
            return item->GetLayerPolarity();
        } );
        break;
    }

    case LAYER_WORKSHEET:
        m_showBorderAndTitleBlock = aNewState;
        break;

    case LAYER_GERBVIEW_GRID:
        SetGridVisibility( aNewState );
        break;

    default:
        wxLogDebug( wxT( "GERBVIEW_FRAME::SetElementVisibility(): bad arg %d" ), aLayerID );
    }

    if( dcodes_changed )
    {
        auto view = GetGalCanvas()->GetView();

        for( int i = 0; i < GERBER_DRAWLAYERS_COUNT; i++ )
        {
            int layer = GERBER_DRAW_LAYER( i );
            int dcode_layer = GERBER_DCODE_LAYER( layer );
            view->SetLayerVisible( dcode_layer, aNewState && view->IsLayerVisible( layer ) );
        }
    }

    applyDisplaySettingsToGAL();
    m_LayersManager->SetRenderState( aLayerID, aNewState );
}


void GERBVIEW_FRAME::applyDisplaySettingsToGAL()
{
    auto view = GetGalCanvas()->GetView();
    auto painter = static_cast<KIGFX::GERBVIEW_PAINTER*>( view->GetPainter() );
    KIGFX::GERBVIEW_RENDER_SETTINGS* settings = painter->GetSettings();
    settings->LoadDisplayOptions( &m_DisplayOptions );

    settings->ImportLegacyColors( m_colorsSettings );

    view->MarkTargetDirty( KIGFX::TARGET_NONCACHED );
}


int GERBVIEW_FRAME::getNextAvailableLayer( int aLayer ) const
{
    int layer = aLayer;

    for( unsigned i = 0; i < ImagesMaxCount(); ++i )
    {
        const GERBER_FILE_IMAGE* gerber = GetGbrImage( layer );

        if( gerber == NULL )    // this graphic layer is available: use it
            return layer;

        ++layer;                // try next graphic layer

        if( layer >= (int)ImagesMaxCount() )
            layer = 0;
    }

    return NO_AVAILABLE_LAYERS;
}


void GERBVIEW_FRAME::syncLayerWidget()
{
    m_LayersManager->SelectLayer( GetActiveLayer() );
}


void GERBVIEW_FRAME::syncLayerBox( bool aRebuildLayerBox )
{
    if( aRebuildLayerBox )
        m_SelLayerBox->Resync();

    m_SelLayerBox->SetSelection( GetActiveLayer() );

    int dcodeSelected = -1;
    GERBER_FILE_IMAGE*   gerber = GetGbrImage( GetActiveLayer() );

    if( gerber )
        dcodeSelected = gerber->m_Selected_Tool;

    if( m_DCodeSelector )
    {
        updateDCodeSelectBox();
        m_DCodeSelector->SetDCodeSelection( dcodeSelected );
        m_DCodeSelector->Enable( gerber != NULL );
    }
}


void GERBVIEW_FRAME::Liste_D_Codes()
{
    int             ii, jj;
    wxString        Line;
    wxArrayString   list;
    double          scale = GetUserUnits() == INCHES ? IU_PER_MILS * 1000 : IU_PER_MM;
    int       curr_layer = GetActiveLayer();

    for( int layer = 0; layer < (int)ImagesMaxCount(); ++layer )
    {
        GERBER_FILE_IMAGE* gerber = GetGbrImage( layer );

        if( gerber == NULL )
            continue;

        if( gerber->GetDcodesCount() == 0 )
            continue;

        if( layer == curr_layer )
            Line.Printf( wxT( "*** Active layer (%2.2d) ***" ), layer + 1 );
        else
            Line.Printf( wxT( "*** layer %2.2d  ***" ), layer + 1 );

        list.Add( Line );

        const char* units = GetUserUnits() == INCHES ? "\"" : "mm";

        for( ii = 0, jj = 1; ii < TOOLS_MAX_COUNT; ii++ )
        {
            D_CODE* pt_D_code = gerber->GetDCODE( ii + FIRST_DCODE );

            if( pt_D_code == NULL )
                continue;

            if( !pt_D_code->m_InUse && !pt_D_code->m_Defined )
                continue;

            Line.Printf( wxT( "tool %2.2d:   D%2.2d   V %.4f %s  H %.4f %s   %s  attribute '%s'" ),
                         jj,
                         pt_D_code->m_Num_Dcode,
                         pt_D_code->m_Size.y / scale, units,
                         pt_D_code->m_Size.x / scale, units,
                         D_CODE::ShowApertureType( pt_D_code->m_Shape ),
                         pt_D_code->m_AperFunction.IsEmpty()? wxT( "none" ) : GetChars( pt_D_code->m_AperFunction )
                         );

            if( !pt_D_code->m_Defined )
                Line += wxT( " (not defined)" );

            if( pt_D_code->m_InUse )
                Line += wxT( " (in use)" );

            list.Add( Line );
            jj++;
        }
    }

    wxSingleChoiceDialog    dlg( this, wxEmptyString, _( "D Codes" ), list, (void**) NULL,
                                 wxCHOICEDLG_STYLE & ~wxCANCEL );

    dlg.ShowModal();
}


void GERBVIEW_FRAME::SortLayersByX2Attributes()
{
    auto remapping = GetImagesList()->SortImagesByZOrder();

    ReFillLayerWidget();
    syncLayerBox( true );

    std::unordered_map<int, int> view_remapping;

    for( auto it : remapping )
    {
        view_remapping[ GERBER_DRAW_LAYER( it.first) ] = GERBER_DRAW_LAYER( it.second );
        view_remapping[ GERBER_DCODE_LAYER( GERBER_DRAW_LAYER( it.first) ) ] =
            GERBER_DCODE_LAYER( GERBER_DRAW_LAYER( it.second ) );
    }

    GetGalCanvas()->GetView()->ReorderLayerData( view_remapping );
    GetCanvas()->Refresh();
}


void GERBVIEW_FRAME::UpdateDisplayOptions( const GBR_DISPLAY_OPTIONS& aOptions )
{
    bool update_flashed =   ( m_DisplayOptions.m_DisplayFlashedItemsFill !=
                              aOptions.m_DisplayFlashedItemsFill );
    bool update_lines =     ( m_DisplayOptions.m_DisplayLinesFill !=
                              aOptions.m_DisplayLinesFill );
    bool update_polygons =  ( m_DisplayOptions.m_DisplayPolygonsFill !=
                              aOptions.m_DisplayPolygonsFill );

    m_DisplayOptions = aOptions;

    applyDisplaySettingsToGAL();

    auto view = GetGalCanvas()->GetView();

    if( update_flashed )
    {
        view->UpdateAllItemsConditionally( KIGFX::REPAINT,
                                           []( KIGFX::VIEW_ITEM* aItem ) {
            auto item = static_cast<GERBER_DRAW_ITEM*>( aItem );

            switch( item->m_Shape )
            {
            case GBR_SPOT_CIRCLE:
            case GBR_SPOT_RECT:
            case GBR_SPOT_OVAL:
            case GBR_SPOT_POLY:
            case GBR_SPOT_MACRO:
                return true;

            default:
                return false;
            }
        } );
    }
    else if( update_lines )
    {
        view->UpdateAllItemsConditionally( KIGFX::REPAINT,
                                           []( KIGFX::VIEW_ITEM* aItem ) {
            auto item = static_cast<GERBER_DRAW_ITEM*>( aItem );

            switch( item->m_Shape )
            {
            case GBR_CIRCLE:
            case GBR_ARC:
            case GBR_SEGMENT:
                return true;

            default:
                return false;
            }
        } );
    }
    else if( update_polygons )
    {
        view->UpdateAllItemsConditionally( KIGFX::REPAINT,
                                           []( KIGFX::VIEW_ITEM* aItem ) {
            auto item = static_cast<GERBER_DRAW_ITEM*>( aItem );

            return ( item->m_Shape == GBR_POLYGON );
        } );
    }

    view->UpdateAllItems( KIGFX::COLOR );

    m_canvas->Refresh( true );
}


void GERBVIEW_FRAME::UpdateTitleAndInfo()
{
    GERBER_FILE_IMAGE* gerber = GetGbrImage( GetActiveLayer() );

    // Display the gerber filename
    if( gerber == NULL )
    {
        SetTitle( "GerbView" );

        SetStatusText( wxEmptyString, 0 );

        wxString info;
        info.Printf( _( "Drawing layer %d not in use" ), GetActiveLayer() + 1 );
        m_TextInfo->SetValue( info );

        if( EnsureTextCtrlWidth( m_TextInfo, &info ) )  // Resized
           m_auimgr.Update();

        ClearMsgPanel();
        return;
    }
    else
    {
        wxString title;
        title.Printf( _( "GerbView" ) + wxT( " \u2014 %s%s" ),
                      gerber->m_FileName,
                      gerber->m_IsX2_file ? _( " (with X2 attributes)" )
                                          : wxString( wxEmptyString ) );
        SetTitle( title );

        gerber->DisplayImageInfo( this );

        // Display Image Name and Layer Name (from the current gerber data):
        wxString status;
        status.Printf( _( "Image name: \"%s\"  Layer name: \"%s\"" ),
                       gerber->m_ImageName,
                       gerber->GetLayerParams().m_LayerName );
        SetStatusText( status, 0 );

        // Display data format like fmt in X3.4Y3.4 no LZ or fmt mm X2.3 Y3.5 no TZ in main toolbar
        wxString info;
        info.Printf( wxT( "fmt: %s X%d.%d Y%d.%d no %cZ" ),
                     gerber->m_GerbMetric ? wxT( "mm" ) : wxT( "in" ),
                     gerber->m_FmtLen.x - gerber->m_FmtScale.x,
                     gerber->m_FmtScale.x,
                     gerber->m_FmtLen.y - gerber->m_FmtScale.y,
                     gerber->m_FmtScale.y,
                     gerber->m_NoTrailingZeros ? 'T' : 'L' );

        if( gerber->m_IsX2_file )
            info << wxT(" ") << _( "X2 attr" );

        m_TextInfo->SetValue( info );

        if( EnsureTextCtrlWidth( m_TextInfo, &info ) )  // Resized
            m_auimgr.Update();
    }
}


bool GERBVIEW_FRAME::IsElementVisible( int aLayerID ) const
{
    switch( aLayerID )
    {
    case LAYER_DCODES:
        return m_DisplayOptions.m_DisplayDCodes;

    case LAYER_NEGATIVE_OBJECTS:
        return m_DisplayOptions.m_DisplayNegativeObjects;

    case LAYER_GERBVIEW_GRID:
        return IsGridVisible();

    case LAYER_WORKSHEET:
        return m_showBorderAndTitleBlock;

    case LAYER_PCB_BACKGROUND:
        return true;

    default:
        wxLogDebug( wxT( "GERBVIEW_FRAME::IsElementVisible(): bad arg %d" ), aLayerID );
    }

    return true;
}


long GERBVIEW_FRAME::GetVisibleLayers() const
{
    long layerMask = 0;

    if( auto canvas = GetGalCanvas() )
    {
        // NOTE: This assumes max 32 drawlayers!
        for( int i = 0; i < GERBER_DRAWLAYERS_COUNT; i++ )
        {
            if( canvas->GetView()->IsLayerVisible( GERBER_DRAW_LAYER( i ) ) )
                layerMask |= ( 1 << i );
        }

        return layerMask;
    }
    else
    {
        return -1;
    }
}


void GERBVIEW_FRAME::SetVisibleLayers( long aLayerMask )
{
    if( auto canvas = GetGalCanvas() )
    {
        // NOTE: This assumes max 32 drawlayers!
        for( int i = 0; i < GERBER_DRAWLAYERS_COUNT; i++ )
        {
            bool v = ( aLayerMask & ( 1 << i ) );
            int layer = GERBER_DRAW_LAYER( i );
            canvas->GetView()->SetLayerVisible( layer, v );
            canvas->GetView()->SetLayerVisible( GERBER_DCODE_LAYER( layer ),
                                                m_DisplayOptions.m_DisplayDCodes && v );
        }
    }
}


bool GERBVIEW_FRAME::IsLayerVisible( int aLayer ) const
{
    if( ! m_DisplayOptions.m_IsPrinting )
        return m_LayersManager->IsLayerVisible( aLayer );
    else
        return GetGerberLayout()->IsLayerPrintable( aLayer );
}


COLOR4D GERBVIEW_FRAME::GetVisibleElementColor( int aLayerID )
{
    COLOR4D color = COLOR4D::UNSPECIFIED;

    switch( aLayerID )
    {
    case LAYER_NEGATIVE_OBJECTS:
    case LAYER_DCODES:
    case LAYER_WORKSHEET:
    case LAYER_PCB_BACKGROUND:
        color = m_colorsSettings->GetItemColor( aLayerID );
        break;

    case LAYER_GERBVIEW_GRID:
        color = GetGridColor();
        break;

    default:
        wxLogDebug( wxT( "GERBVIEW_FRAME::GetVisibleElementColor(): bad arg %d" ), aLayerID );
    }

    return color;
}


void GERBVIEW_FRAME::SetGridVisibility( bool aVisible )
{
    EDA_DRAW_FRAME::SetGridVisibility( aVisible );
    m_LayersManager->SetRenderState( LAYER_GERBVIEW_GRID, aVisible );
}


void GERBVIEW_FRAME::SetVisibleElementColor( int aLayerID, COLOR4D aColor )
{
    switch( aLayerID )
    {
    case LAYER_NEGATIVE_OBJECTS:
    case LAYER_DCODES:
    case LAYER_WORKSHEET:
        m_colorsSettings->SetItemColor( aLayerID, aColor );
        break;

    case LAYER_GERBVIEW_GRID:
        SetGridColor( aColor );
        m_colorsSettings->SetItemColor( aLayerID, aColor );
        break;

    case LAYER_PCB_BACKGROUND:
        SetDrawBgColor( aColor );
        m_colorsSettings->SetItemColor( aLayerID, aColor );
        break;

    default:
        wxLogDebug( wxT( "GERBVIEW_FRAME::SetVisibleElementColor(): bad arg %d" ), aLayerID );
    }
}

COLOR4D GERBVIEW_FRAME::GetNegativeItemsColor()
{
    if( IsElementVisible( LAYER_NEGATIVE_OBJECTS ) )
        return GetVisibleElementColor( LAYER_NEGATIVE_OBJECTS );
    else
        return GetDrawBgColor();
}


COLOR4D GERBVIEW_FRAME::GetLayerColor( int aLayer ) const
{
    return m_colorsSettings->GetLayerColor( aLayer );
}


void GERBVIEW_FRAME::SetLayerColor( int aLayer, COLOR4D aColor )
{
    m_colorsSettings->SetLayerColor( aLayer, aColor );
    applyDisplaySettingsToGAL();
}


int GERBVIEW_FRAME::GetActiveLayer()
{
    return ( (GBR_SCREEN*) GetScreen() )->m_Active_Layer;
}


void GERBVIEW_FRAME::SetActiveLayer( int aLayer, bool doLayerWidgetUpdate )
{
    ( (GBR_SCREEN*) GetScreen() )->m_Active_Layer = aLayer;

    if( doLayerWidgetUpdate )
        m_LayersManager->SelectLayer( GetActiveLayer() );

    UpdateTitleAndInfo();

    if( IsGalCanvasActive() )
    {
        m_toolManager->RunAction( GERBVIEW_ACTIONS::layerChanged );       // notify other tools
        GetGalCanvas()->SetFocus();                 // otherwise hotkeys are stuck somewhere

        GetGalCanvas()->SetHighContrastLayer( GERBER_DRAW_LAYER( aLayer ) );

        GetGalCanvas()->Refresh();
    }
}


void GERBVIEW_FRAME::SetPageSettings( const PAGE_INFO& aPageSettings )
{
    m_paper = aPageSettings;
    GBR_SCREEN* screen = static_cast<GBR_SCREEN*>( GetScreen() );

    if( screen )
        screen->InitDataPoints( aPageSettings.GetSizeIU() );

    if( IsGalCanvasActive() )
    {
        auto drawPanel = static_cast<GERBVIEW_DRAW_PANEL_GAL*>( GetGalCanvas() );

        // Prepare worksheet template
        auto worksheet =
                new KIGFX::WORKSHEET_VIEWITEM( IU_PER_MILS, &GetPageSettings(), &GetTitleBlock() );

        if( screen != NULL )
        {
            worksheet->SetSheetNumber( 1 );
            worksheet->SetSheetCount( 1 );
        }

        // PCB_DRAW_PANEL_GAL takes ownership of the worksheet
        drawPanel->SetWorksheet( worksheet );
    }
}


const PAGE_INFO& GERBVIEW_FRAME::GetPageSettings() const
{
    return m_paper;
}


const wxSize GERBVIEW_FRAME::GetPageSizeIU() const
{
    // this function is only needed because EDA_DRAW_FRAME is not compiled
    // with either -DPCBNEW or -DEESCHEMA, so the virtual is used to route
    // into an application specific source file.
    return GetPageSettings().GetSizeIU();
}


const TITLE_BLOCK& GERBVIEW_FRAME::GetTitleBlock() const
{
    wxASSERT( m_gerberLayout );
    return m_gerberLayout->GetTitleBlock();
}


void GERBVIEW_FRAME::SetTitleBlock( const TITLE_BLOCK& aTitleBlock,
    const TITLE_BLOCK_EXPORT_OPTIONS& aExportOpts )
{
    wxASSERT( m_gerberLayout );
    m_gerberLayout->SetTitleBlock( aTitleBlock );
}


const wxPoint& GERBVIEW_FRAME::GetAuxOrigin() const
{
    wxASSERT( m_gerberLayout );
    return m_gerberLayout->GetAuxOrigin();
}


void GERBVIEW_FRAME::SetAuxOrigin( const wxPoint& aPosition )
{
    wxASSERT( m_gerberLayout );
    m_gerberLayout->SetAuxOrigin( aPosition );
}


void GERBVIEW_FRAME::SetCurItem( GERBER_DRAW_ITEM* aItem, bool aDisplayInfo )
{
    GetScreen()->SetCurItem( aItem );

    if( aItem )
    {
        if( aDisplayInfo )
        {
            MSG_PANEL_ITEMS items;
            aItem->GetMsgPanelInfo( m_UserUnits, items );
            SetMsgPanel( items );
        }
    }
    else
    {
        EraseMsgBox();
    }
}


void GERBVIEW_FRAME::SetGridColor( COLOR4D aColor )
{
    if( IsGalCanvasActive() )
        GetGalCanvas()->GetGAL()->SetGridColor( aColor );

    m_gridColor = aColor;
}


EDA_RECT GERBVIEW_FRAME::GetGerberLayoutBoundingBox()
{
    GetGerberLayout()->ComputeBoundingBox();
    return GetGerberLayout()->GetBoundingBox();
}


void GERBVIEW_FRAME::UpdateStatusBar()
{
    EDA_DRAW_FRAME::UpdateStatusBar();

    GBR_SCREEN* screen = (GBR_SCREEN*) GetScreen();

    if( !screen )
        return;

    int dx;
    int dy;
    double dXpos;
    double dYpos;
    wxString line;

    if( m_DisplayOptions.m_DisplayPolarCood )  // display relative polar coordinates
    {
        double       theta, ro;

        dx = GetCrossHairPosition().x - screen->m_O_Curseur.x;
        dy = GetCrossHairPosition().y - screen->m_O_Curseur.y;

        // atan2 in the 0,0 case returns 0
        theta = RAD2DEG( atan2( (double) -dy, (double) dx ) );

        ro = hypot( dx, dy );
        wxString formatter;
        switch( GetUserUnits() )
        {
        case INCHES:         formatter = wxT( "r %.6f  theta %.1f" ); break;
        case MILLIMETRES:    formatter = wxT( "r %.5f  theta %.1f" ); break;
        case UNSCALED_UNITS: formatter = wxT( "r %f  theta %f" );     break;
        case DEGREES:        wxASSERT( false );                       break;
        }

        line.Printf( formatter, To_User_Unit( GetUserUnits(), ro ), theta );

        SetStatusText( line, 3 );
    }

    // Display absolute coordinates:
    dXpos = To_User_Unit( GetUserUnits(), GetCrossHairPosition().x );
    dYpos = To_User_Unit( GetUserUnits(), GetCrossHairPosition().y );

    wxString absformatter;
    wxString relformatter;

    switch( GetUserUnits() )
    {
    case INCHES:
        absformatter = wxT( "X %.6f  Y %.6f" );
        relformatter = wxT( "dx %.6f  dy %.6f  dist %.4f" );
        break;

    case MILLIMETRES:
        absformatter = wxT( "X %.5f  Y %.5f" );
        relformatter = wxT( "dx %.5f  dy %.5f  dist %.3f" );
        break;

    case UNSCALED_UNITS:
        absformatter = wxT( "X %f  Y %f" );
        relformatter = wxT( "dx %f  dy %f  dist %f" );
        break;

    case DEGREES:
        wxASSERT( false );
        break;
    }

    line.Printf( absformatter, dXpos, dYpos );
    SetStatusText( line, 2 );

    if( !m_DisplayOptions.m_DisplayPolarCood )  // display relative cartesian coordinates
    {
        // Display relative coordinates:
        dx = GetCrossHairPosition().x - screen->m_O_Curseur.x;
        dy = GetCrossHairPosition().y - screen->m_O_Curseur.y;
        dXpos = To_User_Unit( GetUserUnits(), dx );
        dYpos = To_User_Unit( GetUserUnits(), dy );

        // We already decided the formatter above
        line.Printf( relformatter, dXpos, dYpos, hypot( dXpos, dYpos ) );
        SetStatusText( line, 3 );
    }
}


const wxString GERBVIEW_FRAME::GetZoomLevelIndicator() const
{
    return EDA_DRAW_FRAME::GetZoomLevelIndicator();
}


GERBER_FILE_IMAGE* GERBVIEW_FRAME::GetGbrImage( int aIdx ) const
{
    return m_gerberLayout->GetImagesList()->GetGbrImage( aIdx );
}


unsigned GERBVIEW_FRAME::ImagesMaxCount() const
{
    return m_gerberLayout->GetImagesList()->ImagesMaxCount();
}


void GERBVIEW_FRAME::unitsChangeRefresh()
{   // Called on units change (see EDA_DRAW_FRAME)
    EDA_DRAW_FRAME::unitsChangeRefresh();
    updateDCodeSelectBox();
    updateGridSelectBox();
}


void GERBVIEW_FRAME::UseGalCanvas( bool aEnable )
{
    EDA_DRAW_FRAME::UseGalCanvas( aEnable );

    EDA_DRAW_PANEL_GAL* galCanvas = GetGalCanvas();

    if( m_toolManager )
        m_toolManager->SetEnvironment( m_gerberLayout, GetGalCanvas()->GetView(),
                                       GetGalCanvas()->GetViewControls(), this );

    if( aEnable )
    {
        if( m_toolManager )
            m_toolManager->ResetTools( TOOL_BASE::GAL_SWITCH );

        m_colorsSettings->SetLegacyMode( false );

        galCanvas->GetGAL()->SetGridColor( GetLayerColor( LAYER_GERBVIEW_GRID ) );

        SetPageSettings( GetPageSettings() );

        galCanvas->GetView()->RecacheAllItems();
        galCanvas->SetEventDispatcher( m_toolDispatcher );
        galCanvas->StartDrawing();
    }
    else
    {
        if( m_toolManager )
            m_toolManager->ResetTools( TOOL_BASE::GAL_SWITCH );

        galCanvas->StopDrawing();

        // Redirect all events to the legacy canvas
        galCanvas->SetEventDispatcher( NULL );

        m_colorsSettings->SetLegacyMode( true );
        m_canvas->Refresh();
    }

    m_LayersManager->ReFill();
    m_LayersManager->ReFillRender();

    ReCreateOptToolbar();
    ReCreateMenuBar();
}


void GERBVIEW_FRAME::setupTools()
{
    // Create the manager and dispatcher & route draw panel events to the dispatcher
    m_toolManager = new TOOL_MANAGER;
    m_toolManager->SetEnvironment( m_gerberLayout, GetGalCanvas()->GetView(),
                                   GetGalCanvas()->GetViewControls(), this );
    m_actions = new GERBVIEW_ACTIONS();
    m_toolDispatcher = new TOOL_DISPATCHER( m_toolManager, m_actions );

    // Register tools
    m_actions->RegisterAllTools( m_toolManager );
    m_toolManager->InitTools();

    // Run the selection tool, it is supposed to be always active
    m_toolManager->InvokeTool( "gerbview.InteractiveSelection" );
}


void GERBVIEW_FRAME::updateGridSelectBox()
{
    UpdateStatusBar();
    DisplayUnitsMsg();

    if( m_gridSelectBox == NULL )
        return;

    // Update grid values with the current units setting.
    m_gridSelectBox->Clear();
    wxArrayString gridsList;
    int icurr = GetScreen()->BuildGridsChoiceList( gridsList, GetUserUnits() != INCHES );

    for( size_t i = 0; i < GetScreen()->GetGridCount(); i++ )
    {
        GRID_TYPE& grid = GetScreen()->GetGrid( i );
        m_gridSelectBox->Append( gridsList[i], (void*) &grid.m_CmdId );
    }

    m_gridSelectBox->SetSelection( icurr );
}


void GERBVIEW_FRAME::updateZoomSelectBox()
{
    if( m_zoomSelectBox == NULL )
        return;

    wxString msg;

    m_zoomSelectBox->Clear();
    m_zoomSelectBox->Append( _( "Zoom Auto" ) );
    m_zoomSelectBox->SetSelection( 0 );

    for( unsigned i = 0;  i < GetScreen()->m_ZoomList.size();  ++i )
    {
        msg = _( "Zoom " );

        double level =  m_zoomLevelCoeff / (double)GetScreen()->m_ZoomList[i];
        wxString value = wxString::Format( wxT( "%.2f" ), level );
        msg += value;

        m_zoomSelectBox->Append( msg );

        if( GetScreen()->GetZoom() == GetScreen()->m_ZoomList[i] )
            m_zoomSelectBox->SetSelection( i + 1 );
    }
}


void GERBVIEW_FRAME::OnUpdateSelectGrid( wxUpdateUIEvent& aEvent )
{
    // No need to update the grid select box if it doesn't exist or the grid setting change
    // was made using the select box.
    if( m_gridSelectBox == NULL || m_auxiliaryToolBar == NULL )
        return;

    int select = wxNOT_FOUND;

    for( size_t i = 0; i < GetScreen()->GetGridCount(); i++ )
    {
        if( GetScreen()->GetGridCmdId() == GetScreen()->GetGrid( i ).m_CmdId )
        {
            select = (int) i;
            break;
        }
    }

    if( select != m_gridSelectBox->GetSelection() )
        m_gridSelectBox->SetSelection( select );
}


void GERBVIEW_FRAME::OnUpdateSelectZoom( wxUpdateUIEvent& aEvent )
{
    if( m_zoomSelectBox == NULL || m_auxiliaryToolBar == NULL )
        return;

    int current = 0;    // display Auto if no match found

    // check for a match within 1%
    double zoom = IsGalCanvasActive() ? GetGalCanvas()->GetLegacyZoom() : GetScreen()->GetZoom();

    for( unsigned i = 0; i < GetScreen()->m_ZoomList.size(); i++ )
    {
        if( std::fabs( zoom - GetScreen()->m_ZoomList[i] ) < ( zoom / 100.0 ) )
        {
            current = i + 1;
            break;
        }
    }

    if( current != m_zoomSelectBox->GetSelection() )
        m_zoomSelectBox->SetSelection( current );
}


void GERBVIEW_FRAME::CommonSettingsChanged()
{
    EDA_DRAW_FRAME::CommonSettingsChanged();

    ReCreateHToolbar();
    ReCreateOptToolbar();
    ReCreateAuxiliaryToolbar();
    Layout();
    SendSizeEvent();
}
