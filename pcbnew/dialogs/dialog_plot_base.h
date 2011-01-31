///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __dialog_plot_base__
#define __dialog_plot_base__

#include <wx/intl.h>

#include <wx/string.h>
#include <wx/radiobox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_PLOT_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_PLOT_BASE : public wxDialog 
{
	private:
	
	protected:
		enum
		{
			ID_SAVE_OPT_PLOT = 1000,
			ID_CREATE_DRILL_FILE,
			ID_ALLOW_PRINT_PAD_ON_SILKSCREEN,
			ID_PRINT_REF,
			ID_MIROR_OPT,
		};
		
		wxRadioBox* m_plotFormatOpt;
		wxButton* m_plotButton;
		wxButton* m_buttonSaveOpt;
		wxButton* m_buttonDrill;
		wxButton* m_buttonQuit;
		wxStaticBoxSizer* m_CopperLayersBoxSizer;
		wxStaticBoxSizer* m_TechnicalLayersBoxSizer;
		wxCheckBox* m_plotSheetRef;
		wxCheckBox* m_plotPads_on_Silkscreen;
		wxCheckBox* m_plotModuleValueOpt;
		wxCheckBox* m_plotModuleRefOpt;
		wxCheckBox* m_plotTextOther;
		wxCheckBox* m_plotInvisibleText;
		wxCheckBox* m_plotNoViaOnMaskOpt;
		wxCheckBox* m_plotMirrorOpt;
		wxStaticText* m_staticText11;
		wxChoice* m_drillShapeOpt;
		wxStaticText* m_staticText12;
		wxChoice* m_scaleOpt;
		wxStaticText* m_staticText13;
		wxChoice* m_plotModeOpt;
		wxStaticText* m_textDefaultPenSize;
		wxTextCtrl* m_linesWidth;
		wxCheckBox* m_useGerberExtensions;
		wxCheckBox* m_excludeEdgeLayerOpt;
		wxCheckBox* m_subtractMaskFromSilk;
		wxCheckBox* m_useAuxOriginCheckBox;
		wxStaticText* m_textPenSize;
		wxTextCtrl* m_HPGLPenSizeOpt;
		wxStaticText* m_textPenOvr;
		wxTextCtrl* m_HPGLPenOverlayOpt;
		wxStaticText* m_textPenSpeed;
		wxTextCtrl* m_HPGLPenSpeedOpt;
		wxStaticText* m_staticText7;
		wxTextCtrl* m_fineAdjustXscaleOpt;
		wxStaticText* m_staticText8;
		wxTextCtrl* m_fineAdjustYscaleOpt;
		wxCheckBox* m_plotPSNegativeOpt;
		wxStaticText* m_staticTextDir;
		wxTextCtrl* m_outputDirectoryName;
		wxButton* m_browseButton;
		wxStaticText* m_staticText2;
		wxTextCtrl* m_messagesBox;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnCloseWindow( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void SetPlotFormat( wxCommandEvent& event ) { event.Skip(); }
		virtual void Plot( wxCommandEvent& event ) { event.Skip(); }
		virtual void applyPlotSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void CreateDrillFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSetScaleOpt( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOutputDirectoryBrowseClicked( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_PLOT_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Plot"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,616 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
		~DIALOG_PLOT_BASE();
	
};

#endif //__dialog_plot_base__
