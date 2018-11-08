/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2010-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file hotkeys_basic.cpp
 * @brief Some functions to handle hotkeys in KiCad
 */

#include <fctsys.h>
#include <kiface_i.h>
#include <hotkeys_basic.h>
#include <id.h>
#include <confirm.h>
#include <kicad_string.h>
#include <gestfich.h>
#include <eda_base_frame.h>
#include <macros.h>
#include <menus_helpers.h>
#include <draw_frame.h>

#include <tool/tool_manager.h>

#include "dialogs/dialog_hotkey_list.h"

#include <wx/apptrait.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>

#define HOTKEYS_CONFIG_KEY wxT( "Keys" )

wxString g_CommonSectionTag( wxT( "[common]" ) );


/* Class to handle hotkey commands hotkeys have a default value
 * This class allows the real key code changed by user from a key code list
 * file.
 */

EDA_HOTKEY::EDA_HOTKEY( const wxChar* infomsg, int idcommand, int keycode, int idmenuevent ) :
            m_defaultKeyCode( keycode ), m_KeyCode( keycode ), m_InfoMsg( infomsg ),
            m_Idcommand( idcommand ), m_IdMenuEvent( idmenuevent )
{
}


EDA_HOTKEY::EDA_HOTKEY( const EDA_HOTKEY* base )
{
    m_defaultKeyCode = base->m_defaultKeyCode;  // initialize default key code
    m_KeyCode     = base->m_KeyCode;
    m_InfoMsg     = base->m_InfoMsg;
    m_Idcommand   = base->m_Idcommand;
    m_IdMenuEvent = base->m_IdMenuEvent;
}


EDA_HOTKEY_CLIENT_DATA::~EDA_HOTKEY_CLIENT_DATA()
{
}


/* class to handle the printable name and the keycode
 */
struct hotkey_name_descr
{
    const wxChar* m_Name;
    int           m_KeyCode;
};

/* table giving the hotkey name from the hotkey code, for special keys
 * Note : when modifiers (ATL, SHIFT, CTRL) do not modify
 * the code of the key, do need to enter the modified key code
 * For instance wxT( "F1" ), WXK_F1 handle F1, AltF1, CtrlF1 ...
 * Key names are:
 *        "Space","Ctrl+Space","Alt+Space" or
 *      "Alt+A","Ctrl+F1", ...
 */
#define KEY_NON_FOUND -1
static struct hotkey_name_descr hotkeyNameList[] =
{
    { wxT( "F1" ),           WXK_F1                                                   },
    { wxT( "F2" ),           WXK_F2                                                   },
    { wxT( "F3" ),           WXK_F3                                                   },
    { wxT( "F4" ),           WXK_F4                                                   },
    { wxT( "F5" ),           WXK_F5                                                   },
    { wxT( "F6" ),           WXK_F6                                                   },
    { wxT( "F7" ),           WXK_F7                                                   },
    { wxT( "F8" ),           WXK_F8                                                   },
    { wxT( "F9" ),           WXK_F9                                                   },
    { wxT( "F10" ),          WXK_F10                                                  },
    { wxT( "F11" ),          WXK_F11                                                  },
    { wxT( "F12" ),          WXK_F12                                                  },

    { wxT( "Esc" ),          WXK_ESCAPE                                               },
    { wxT( "Del" ),          WXK_DELETE                                               },
    { wxT( "Tab" ),          WXK_TAB                                                  },
    { wxT( "Back" ),         WXK_BACK                                                 },
    { wxT( "Ins" ),          WXK_INSERT                                               },

    { wxT( "Home" ),         WXK_HOME                                                 },
    { wxT( "End" ),          WXK_END                                                  },
    { wxT( "PgUp" ),         WXK_PAGEUP                                               },
    { wxT( "PgDn" ),         WXK_PAGEDOWN                                             },

    { wxT( "Up" ),           WXK_UP                                                   },
    { wxT( "Down" ),         WXK_DOWN                                                 },
    { wxT( "Left" ),         WXK_LEFT                                                 },
    { wxT( "Right" ),        WXK_RIGHT                                                },

    { wxT( "Return" ),       WXK_RETURN                                               },

    { wxT( "Space" ),        WXK_SPACE                                                },

    { wxT( "<unassigned>" ), 0                                                        },

    // Do not change this line: end of list
    { wxT( "" ),             KEY_NON_FOUND                                            }
};

// name of modifier keys.
// Note: the Ctrl key is Cmd key on Mac OS X.
// However, in wxWidgets defs, the key WXK_CONTROL is the Cmd key,
// so the code using WXK_CONTROL should be ok on any system.
// (on Mac OS X the actual Ctrl key code is WXK_RAW_CONTROL)
#ifdef __WXMAC__
#define USING_MAC_CMD
#endif

#ifdef USING_MAC_CMD
#define MODIFIER_CTRL       wxT( "Cmd+" )
#else
#define MODIFIER_CTRL       wxT( "Ctrl+" )
#endif
#define MODIFIER_CMD_MAC    wxT( "Cmd+" )
#define MODIFIER_CTRL_BASE  wxT( "Ctrl+" )
#define MODIFIER_ALT        wxT( "Alt+" )
#define MODIFIER_SHIFT      wxT( "Shift+" )


/**
 * Function KeyNameFromKeyCode
 * return the key name from the key code
 * Only some wxWidgets key values are handled for function key ( see
 * hotkeyNameList[] )
 * @param aKeycode = key code (ascii value, or wxWidgets value for function keys)
 * @param aIsFound = a pointer to a bool to return true if found, or false. an be nullptr default)
 * @return the key name in a wxString
 */
wxString KeyNameFromKeyCode( int aKeycode, bool* aIsFound )
{
    wxString keyname, modifier, fullkeyname;
    int      ii;
    bool     found = false;

    // Assume keycode of 0 is "unassigned"
    if( (aKeycode & GR_KB_CTRL) != 0 )
        modifier << MODIFIER_CTRL;

    if( (aKeycode & GR_KB_ALT) != 0 )
        modifier << MODIFIER_ALT;

    if( (aKeycode & GR_KB_SHIFT) != 0 )
        modifier << MODIFIER_SHIFT;

    aKeycode &= ~( GR_KB_CTRL | GR_KB_ALT | GR_KB_SHIFT );

    if( (aKeycode > ' ') && (aKeycode < 0x7F ) )
    {
        found   = true;
        keyname.Append( (wxChar)aKeycode );
    }
    else
    {
        for( ii = 0; ; ii++ )
        {
            if( hotkeyNameList[ii].m_KeyCode == KEY_NON_FOUND ) // End of list
            {
                keyname = wxT( "<unknown>" );
                break;
            }

            if( hotkeyNameList[ii].m_KeyCode == aKeycode )
            {
                keyname = hotkeyNameList[ii].m_Name;
                found   = true;
                break;
            }
        }
    }

    if( aIsFound )
        *aIsFound = found;

    fullkeyname = modifier + keyname;
    return fullkeyname;
}


/*
 * helper function use in AddHotkeyName to calculate an accelerator string
 * In some menus, accelerators do not perform exactly the same action as
 * the hotkey that perform a similar action.
 * this is usually the case when this action uses the current mouse position
 * for instance zoom action is ran from the F1 key or the Zoom menu.
 * a zoom uses the mouse position from a hot key and not from the menu
 * In this case, the accelerator if Shift+<hotkey>
 * But for many keys, the Shift modifier is not usable, and the accelerator is Alt+<hotkey>
 */
static void AddModifierToKey( wxString& aFullKey, const wxString & aKey )
{
    if( (aKey.Length() == 1) && (aKey[0] >= 'A')  && (aKey[0] <= 'Z'))
        // We can use Shift+<key> as accelerator and <key> for hot key
        aFullKey << wxT( "\t" ) << MODIFIER_SHIFT << aKey;
    else
        // We must use Alt+<key> as accelerator and <key> for hot key
        aFullKey << wxT( "\t" ) << MODIFIER_ALT << aKey;
}


/* AddHotkeyName
 * Add the key name from the Command id value ( m_Idcommand member value)
 *  aText = a wxString. returns aText + key name
 *  aList = pointer to a EDA_HOTKEY list of commands
 *  aCommandId = Command Id value
 *  aShortCutType = IS_HOTKEY to add <tab><keyname> (shortcuts in menus, same as hotkeys)
 *                  IS_ACCELERATOR to add <tab><Shift+keyname> (accelerators in menus, not hotkeys)
 *                  IS_COMMENT to add <spaces><(keyname)> mainly in tool tips
 *  Return a wxString (aTest + key name) if key found or aText without modification
 */
wxString AddHotkeyName( const wxString& aText, EDA_HOTKEY** aList,
                        int aCommandId, HOTKEY_ACTION_TYPE aShortCutType )
{
    wxString msg = aText;
    wxString keyname;

    if( aList )
        keyname = KeyNameFromCommandId( aList, aCommandId );

    if( !keyname.IsEmpty() )
    {
        switch( aShortCutType )
        {
        case IS_HOTKEY:
            msg << wxT( "\t" ) << keyname;
            break;

        case IS_ACCELERATOR:
            AddModifierToKey( msg, keyname );
            break;

        case IS_COMMENT:
            msg << wxT( " (" ) << keyname << wxT( ")" );
            break;
        }
    }

#ifdef USING_MAC_CMD
    // On OSX, the modifier equivalent to the Ctrl key of PCs
    // is the Cmd key, but in code we should use Ctrl as prefix in menus
    msg.Replace( MODIFIER_CMD_MAC, MODIFIER_CTRL_BASE );
#endif

    return msg;
}


/* AddHotkeyName
 * Add the key name from the Command id value ( m_Idcommand member value)
 *  aText = a wxString. returns aText + key name
 *  aList = pointer to a EDA_HOTKEY_CONFIG DescrList of commands
 *  aCommandId = Command Id value
 *  aShortCutType = IS_HOTKEY to add <tab><keyname> (active shortcuts in menus)
 *                  IS_ACCELERATOR to add <tab><Shift+keyname> (active accelerators in menus)
 *                  IS_COMMENT to add <spaces><(keyname)>
 * Return a wxString (aText + key name) if key found or aText without modification
 */
wxString AddHotkeyName( const wxString&           aText,
                        struct EDA_HOTKEY_CONFIG* aDescList,
                        int                       aCommandId,
                        HOTKEY_ACTION_TYPE        aShortCutType )
{
    wxString     msg = aText;
    wxString     keyname;
    EDA_HOTKEY** list;

    if( aDescList )
    {
        for( ; aDescList->m_HK_InfoList != nullptr; aDescList++ )
        {
            list    = aDescList->m_HK_InfoList;
            keyname = KeyNameFromCommandId( list, aCommandId );

            if( !keyname.IsEmpty() )
            {
                switch( aShortCutType )
                {
                case IS_HOTKEY:
                    msg << wxT( "\t" ) << keyname;
                    break;

                case IS_ACCELERATOR:
                    AddModifierToKey( msg, keyname );
                    break;

                case IS_COMMENT:
                    msg << wxT( " (" ) << keyname << wxT( ")" );
                    break;
                }

                break;
            }
        }
    }

#ifdef USING_MAC_CMD
    // On OSX, the modifier equivalent to the Ctrl key of PCs
    // is the Cmd key, but in code we should use Ctrl as prefix in menus
    msg.Replace( MODIFIER_CMD_MAC, MODIFIER_CTRL_BASE );
#endif

    return msg;
}


/**
 * Function KeyNameFromCommandId
 * return the key name from the Command id value ( m_Idcommand member value)
 * @param aList = pointer to a EDA_HOTKEY list of commands
 * @param aCommandId = Command Id value
 * @return the key name in a wxString
 */
wxString KeyNameFromCommandId( EDA_HOTKEY** aList, int aCommandId )
{
    wxString keyname;

    for( ; *aList != nullptr; aList++ )
    {
        EDA_HOTKEY* hk_decr = *aList;

        if( hk_decr->m_Idcommand == aCommandId )
        {
            keyname = KeyNameFromKeyCode( hk_decr->m_KeyCode );
            break;
        }
    }

    return keyname;
}


/**
 * Function KeyCodeFromKeyName
 * return the key code from its key name
 * Only some wxWidgets key values are handled for function key
 * @param keyname = wxString key name to find in hotkeyNameList[],
 *   like F2 or space or an usual (ascii) char.
 * @return the key code
 */
int KeyCodeFromKeyName( const wxString& keyname )
{
    int ii, keycode = KEY_NON_FOUND;

    // Search for modifiers: Ctrl+ Alt+ and Shift+
    // Note: on Mac OSX, the Cmd key is equiv here to Ctrl
    wxString key = keyname;
    wxString prefix;
    int modifier = 0;

    while( 1 )
    {
        prefix.Empty();

        if( key.StartsWith( MODIFIER_CTRL_BASE ) )
        {
            modifier |= GR_KB_CTRL;
            prefix = MODIFIER_CTRL_BASE;
        }
        else if( key.StartsWith( MODIFIER_CMD_MAC ) )
        {
            modifier |= GR_KB_CTRL;
            prefix = MODIFIER_CMD_MAC;
        }
        else if( key.StartsWith( MODIFIER_ALT ) )
        {
            modifier |= GR_KB_ALT;
            prefix = MODIFIER_ALT;
        }
        else if( key.StartsWith( MODIFIER_SHIFT ) )
        {
            modifier |= GR_KB_SHIFT;
            prefix = MODIFIER_SHIFT;
        }
        else
        {
            break;
        }

        if( !prefix.IsEmpty() )
            key.Remove( 0, prefix.Len() );
    }

    if( (key.length() == 1) && (key[0] > ' ') && (key[0] < 0x7F) )
    {
        keycode = key[0];
        keycode += modifier;
        return keycode;
    }

    for( ii = 0; hotkeyNameList[ii].m_KeyCode != KEY_NON_FOUND; ii++ )
    {
        if( key.CmpNoCase( hotkeyNameList[ii].m_Name ) == 0 )
        {
            keycode = hotkeyNameList[ii].m_KeyCode + modifier;
            break;
        }
    }

    return keycode;
}


/**
 * Function GetDescriptorFromHotkey
 * Return a EDA_HOTKEY * pointer from a key code for OnHotKey() function
 * @param aKey = key code (ascii value, or wxWidgets value for function keys
 * @param aList = pointer to a EDA_HOTKEY list of commands
 * @return the corresponding EDA_HOTKEY pointer from the EDA_HOTKEY List
 */
EDA_HOTKEY* GetDescriptorFromHotkey( int aKey, EDA_HOTKEY** aList )
{
    for( ; *aList != nullptr; aList++ )
    {
        EDA_HOTKEY* hk_decr = *aList;

        if( hk_decr->m_KeyCode == aKey )
            return hk_decr;
    }

    return nullptr;
}


EDA_HOTKEY* GetDescriptorFromCommand( int aCommand, EDA_HOTKEY** aList )
{
    for( ; *aList != nullptr; aList++ )
    {
        EDA_HOTKEY* hk_decr = *aList;

        if( hk_decr->m_Idcommand == aCommand )
            return hk_decr;
    }

    return nullptr;
}


int EDA_BASE_FRAME::WriteHotkeyConfig( struct EDA_HOTKEY_CONFIG* aDescList,
                                       wxString*                 aFullFileName )
{
    wxString msg;
    wxString keyname, infokey;
    FILE* file;

    msg = wxT( "$hotkey list\n" );

    // Print the current hotkey list
    EDA_HOTKEY** list;

    for( ; aDescList->m_HK_InfoList != nullptr; aDescList++ )
    {
        if( aDescList->m_Title )
        {
            msg += wxT( "# " );
            msg += *aDescList->m_Title;
            msg += wxT( "\n" );
        }

        msg += *aDescList->m_SectionTag;
        msg += wxT( "\n" );

        list = aDescList->m_HK_InfoList;

        for( ; *list != nullptr; list++ )
        {
            EDA_HOTKEY* hk_decr = *list;
            msg    += wxT( "shortcut   " );
            keyname = KeyNameFromKeyCode( hk_decr->m_KeyCode );
            AddDelimiterString( keyname );
            infokey = hk_decr->m_InfoMsg;
            AddDelimiterString( infokey );
            msg += keyname + wxT( ":    " ) + infokey + wxT( "\n" );
        }
    }

    msg += wxT( "$Endlist\n" );

    if( aFullFileName )
        file = wxFopen( *aFullFileName, wxT( "wt" ) );
    else
    {
        wxString configName( ConfigBaseName() );
        if( configName == SCH_EDIT_FRAME_NAME  || configName == LIB_EDIT_FRAME_NAME )
            configName = EESCHEMA_HOTKEY_NAME;
        else if( configName == PCB_EDIT_FRAME_NAME  ||
                configName == FOOTPRINT_EDIT_FRAME_NAME )
            configName = PCBNEW_HOTKEY_NAME;

        wxFileName fn( configName );
        fn.SetExt( DEFAULT_HOTKEY_FILENAME_EXT );
        fn.SetPath( GetKicadConfigPath() );
        file = wxFopen( fn.GetFullPath(), wxT( "wt" ) );
    }

    if( file )
    {
        wxFputs( msg, file );
        fclose( file );
    }
    else
    {
        msg.Printf( wxT( "Unable to write file %s" ), GetChars( *aFullFileName ) );
        return 0;
    }

    return 1;
}


int ReadHotkeyConfigFile( const wxString& aFilename, struct EDA_HOTKEY_CONFIG* aDescList,
        const bool aDefaultLocation )
{
    wxFileName fn( aFilename );

    if( aDefaultLocation )
    {
        fn.SetExt( DEFAULT_HOTKEY_FILENAME_EXT );
        fn.SetPath( GetKicadConfigPath() );
    }

    if( !wxFile::Exists( fn.GetFullPath() ) )
        return 0;

    wxFile cfgfile( fn.GetFullPath() );
    if( !cfgfile.IsOpened() )       // There is a problem to open file
        return 0;

    // get length
    cfgfile.SeekEnd();
    wxFileOffset size = cfgfile.Tell();
    cfgfile.Seek( 0 );

    // read data
    std::vector<char> buffer( size );
    cfgfile.Read( buffer.data(), size );
    wxString data( buffer.data(), wxConvUTF8, size );

    // Is this the wxConfig format? If so, remove "Keys=" and parse the newlines.
    if( data.StartsWith( wxT("Keys="), &data ) )
        data.Replace( "\\n", "\n", true );

    // parse
    ParseHotkeyConfig( data, aDescList, aFilename );

    // cleanup
    cfgfile.Close();
    return 1;
}


int ReadHotkeyConfig( const wxString& aAppname, struct EDA_HOTKEY_CONFIG* aDescList )
{
    // For Eeschema and Pcbnew frames, we read the new combined file.
    // For other kifaces, we read the frame-based file
    if( aAppname == LIB_EDIT_FRAME_NAME || aAppname == SCH_EDIT_FRAME_NAME )
    {
        return ReadHotkeyConfigFile( EESCHEMA_HOTKEY_NAME, aDescList );
    }
    else if( aAppname == PCB_EDIT_FRAME_NAME || aAppname == FOOTPRINT_EDIT_FRAME_NAME )
    {
        return ReadHotkeyConfigFile( PCBNEW_HOTKEY_NAME, aDescList );
    }

    return ReadHotkeyConfigFile( aAppname, aDescList );
}


/* Function ParseHotkeyConfig
 * the input format is: shortcut  "key"  "function"
 * lines starting by # are ignored (comments)
 * lines like [xxx] are tags (example: [common] or [libedit] which identify sections
 */
void ParseHotkeyConfig( const wxString&           data,
                        struct EDA_HOTKEY_CONFIG* aDescList,
                        const wxString&           aAppname )
{
    // Read the config
    wxStringTokenizer tokenizer( data, L"\r\n", wxTOKEN_STRTOK );
    EDA_HOTKEY**      CurrentHotkeyList = nullptr;

    while( tokenizer.HasMoreTokens() )
    {
        wxString          line = tokenizer.GetNextToken();
        wxStringTokenizer lineTokenizer( line );

        wxString          line_type = lineTokenizer.GetNextToken();

        if( line_type[0]  == '#' ) //comment
            continue;

        if( line_type[0]  == '[' ) // A tag is found. search infos in list
        {
            CurrentHotkeyList = nullptr;
            EDA_HOTKEY_CONFIG* DList = aDescList;

            for( ; DList->m_HK_InfoList; DList++ )
            {
                if( *DList->m_SectionTag == line_type )
                {
                    CurrentHotkeyList = DList->m_HK_InfoList;
                    break;
                }
            }

            continue;
        }

        // Do not accept hotkey assignments from hotkey files that don't match the application
        if( aAppname == LIB_EDIT_FRAME_NAME && line_type == wxT( "[eeschema]" ) )
            CurrentHotkeyList = nullptr;

        if( aAppname == SCH_EDIT_FRAME_NAME && line_type == wxT( "[libedit]" ) )
            CurrentHotkeyList = nullptr;

        if( aAppname == PCB_EDIT_FRAME_NAME && line_type == wxT( "[footprinteditor]" ) )
            CurrentHotkeyList = nullptr;

        if( aAppname == FOOTPRINT_EDIT_FRAME_NAME && line_type == wxT( "[pcbnew]" ) )
            CurrentHotkeyList = nullptr;

        if( line_type == wxT( "$Endlist" ) )
            break;

        if( line_type != wxT( "shortcut" ) )
            continue;

        if( CurrentHotkeyList == nullptr )
            continue;

        // Get the key name
        lineTokenizer.SetString( lineTokenizer.GetString(), L"\"\r\n\t ", wxTOKEN_STRTOK );
        wxString keyname = lineTokenizer.GetNextToken();

        wxString remainder = lineTokenizer.GetString();

        // Get the command name
        wxString fctname = remainder.AfterFirst( '\"' ).BeforeFirst( '\"' );

        // search the hotkey in current hotkey list
        for( EDA_HOTKEY** list = CurrentHotkeyList; *list != nullptr; list++ )
        {
            EDA_HOTKEY* hk_decr = *list;

            if( hk_decr->m_InfoMsg == fctname )
            {
                int keycode = KeyCodeFromKeyName( keyname );

                if( keycode != KEY_NON_FOUND )   // means the key name is found in list or unassigned
                    hk_decr->m_KeyCode = keycode;

                break;
            }
        }
    }
}


void EDA_BASE_FRAME::ImportHotkeyConfigFromFile( EDA_HOTKEY_CONFIG* aDescList,
                                                 const wxString& aDefaultShortname )
{
    wxString ext  = DEFAULT_HOTKEY_FILENAME_EXT;
    wxString mask = wxT( "*." ) + ext;


    wxString path = GetMruPath();
    wxFileName fn( aDefaultShortname );
    fn.SetExt( DEFAULT_HOTKEY_FILENAME_EXT );

    wxString  filename = EDA_FILE_SELECTOR( _( "Read Hotkey Configuration File:" ),
                                           path,
                                           fn.GetFullPath(),
                                           ext,
                                           mask,
                                           this,
                                           wxFD_OPEN,
                                           true );

    if( filename.IsEmpty() )
        return;

    ::ReadHotkeyConfigFile( filename, aDescList, false );
    WriteHotkeyConfig( aDescList );
    SetMruPath( wxFileName( filename ).GetPath() );
}


void EDA_BASE_FRAME::ExportHotkeyConfigToFile( EDA_HOTKEY_CONFIG* aDescList,
                                               const wxString& aDefaultShortname )
{
    wxString ext  = DEFAULT_HOTKEY_FILENAME_EXT;
    wxString mask = wxT( "*." ) + ext;

#if 0
    wxString path = wxPathOnly( Prj().GetProjectFullName() );
#else
    wxString path = GetMruPath();
#endif
    wxFileName fn( aDefaultShortname );
    fn.SetExt( DEFAULT_HOTKEY_FILENAME_EXT );

    wxString filename = EDA_FILE_SELECTOR( _( "Write Hotkey Configuration File:" ),
                                           path,
                                           fn.GetFullPath(),
                                           ext,
                                           mask,
                                           this,
                                           wxFD_SAVE,
                                           true );

    if( filename.IsEmpty() )
        return;

    WriteHotkeyConfig( aDescList, &filename );
    SetMruPath( wxFileName( filename ).GetPath() );
}

