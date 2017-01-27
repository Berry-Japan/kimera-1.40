/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qvaluevector.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qtooltip.h>
#include "kimeraglobal.h"

const QString PREFIX = "ktbl";
const int  MAX_AVAILABLE_KEYS = 60;

static QValueVector<QString>  FuncName(FID_NumFunctions);

static int KeyAssign[NUM_SETTING_TYPE][MAX_AVAILABLE_KEYS][NUM_OF_CONVSTATUS + 1] = {
  { { Qt::Key_Space,     FID_InsertSpace, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },    // MSIME
    { Qt::CTRL+Qt::Key_Space,  FID_None, FID_ConvertToOneSegment, FID_None, FID_None, FID_None },
    { Qt::SHIFT+Qt::Key_Space, FID_InsertOtherWidthSpace, FID_ConvertAllSegments, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment }, 
    { Qt::CTRL+Qt::SHIFT+Qt::Key_Space,  FID_InsertMultibyteSpace, FID_None, FID_None, FID_None, FID_None },
    { Qt::Key_Return,    FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Enter,     FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_Return, FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_Enter,  FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::SHIFT+Qt::Key_Return, FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::SHIFT+Qt::Key_Enter,  FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Backspace, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_None },
    { Qt::CTRL+Qt::Key_Backspace, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_None },
    { Qt::SHIFT+Qt::Key_Backspace, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_None },
    { Qt::Key_Delete,    FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::Key_Escape,    FID_None, FID_DeleteAll, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::SHIFT+Qt::Key_Escape, FID_None, FID_DeleteAll, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::CTRL+Qt::Key_A, FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_D, FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::CTRL+Qt::Key_E, FID_None, FID_CaretToFirst, FID_PreviousCandidate, FID_PreviousCandidate, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_F, FID_None, FID_CaretToLast, FID_ToLastSegment, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_G, FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_H, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_None },
    { Qt::CTRL+Qt::Key_I, FID_None, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::CTRL+Qt::Key_K, FID_None, FID_CaretBackward, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_L, FID_None, FID_CaretForward, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_M, FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_N, FID_None, FID_CaretToLast, FID_DecideCrntSegment, FID_DecideCrntSegment, FID_DecideCrntSegment },
    { Qt::CTRL+Qt::Key_O, FID_None, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::CTRL+Qt::Key_P, FID_None, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::CTRL+Qt::Key_S, FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_BackwardSegment },
    { Qt::CTRL+Qt::Key_T, FID_None, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu },
    { Qt::CTRL+Qt::Key_U, FID_None, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::CTRL+Qt::Key_X, FID_None, FID_CaretToLast, FID_NextCandidate, FID_NextCandidate, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_Z, FID_None, FID_DeleteAll, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::Key_Home,      FID_None, FID_CaretToFirst, FID_None, FID_None, FID_None },
    { Qt::Key_End,       FID_None, FID_CaretToLast, FID_None, FID_None, FID_None },
    { Qt::Key_Left,      FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_BackwardSegment },
    { Qt::Key_Right,     FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::Key_Up,        FID_None, FID_CaretToFirst, FID_PreviousCandidate, FID_PreviousCandidate, FID_ShortenSegment },
    { Qt::Key_Down,      FID_None, FID_CaretToLast, FID_NextCandidate, FID_NextCandidate, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_Left,   FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_Right,  FID_None, FID_CaretToLast, FID_ToLastSegment, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_Up,     FID_None, FID_CaretToFirst, FID_PreviousCandidate, FID_PreviousCandidate, FID_None },
    { Qt::CTRL+Qt::Key_Down,   FID_None, FID_CaretToLast, FID_DecideCrntSegment, FID_DecideCrntSegment, FID_DecideCrntSegment },
    { Qt::SHIFT+Qt::Key_Left,  FID_None, FID_CaretBackward, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::SHIFT+Qt::Key_Right, FID_None, FID_CaretForward, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::SHIFT+Qt::Key_Up,    FID_None, FID_None, FID_None, FID_PreviousCandidateGroup, FID_None },
    { Qt::SHIFT+Qt::Key_Down,  FID_None, FID_None, FID_None, FID_NextCandidateGroup, FID_None },
    { Qt::Key_PageUp,    FID_None, FID_None, FID_None, FID_PreviousCandidateGroup, FID_None },
    { Qt::Key_PageDown,  FID_None, FID_None, FID_None, FID_NextCandidateGroup, FID_None },    
    { Qt::Key_F6,        FID_None, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::Key_F7,        FID_None, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::Key_F8,        FID_None, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::Key_F9,        FID_None, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::Key_F10,       FID_None, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu },
    { Qt::Key_Henkan,    FID_ReconvertClipboardString, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment }, },

  { { Qt::Key_Space,     FID_InsertSpace, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },   // ATOK
    { Qt::CTRL+Qt::Key_Space,  FID_None, FID_ConvertToOneSegment, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::SHIFT+Qt::Key_Space, FID_InsertOtherWidthSpace, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::Key_Return,    FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Enter,     FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::SHIFT+Qt::Key_Return,    FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::SHIFT+Qt::Key_Enter,     FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Backspace, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::Key_Delete,    FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_Delete,     FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::SHIFT+Qt::Key_Delete,    FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::Key_Escape,    FID_None, FID_DeleteAll, FID_DeleteAll, FID_DeleteAll, FID_DeleteAll },
    { Qt::SHIFT+Qt::Key_Escape, FID_None, FID_DeleteAll, FID_DeleteAll, FID_DeleteAll, FID_DeleteAll },
    { Qt::Key_Insert,    FID_None, FID_None, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::CTRL+Qt::Key_Insert, FID_None, FID_None, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::SHIFT+Qt::Key_Insert, FID_None, FID_None, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::CTRL+Qt::Key_A, FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_ToFirstSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_D, FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_E, FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_None },
    { Qt::CTRL+Qt::Key_F, FID_None, FID_CaretToLast, FID_ToLastSegment, FID_ToLastSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_G, FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_H, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::CTRL+Qt::Key_I, FID_None, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::CTRL+Qt::Key_K, FID_None, FID_CaretBackward, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_L, FID_None, FID_CaretForward, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_M, FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_N, FID_None, FID_DecideAllSegments, FID_DecideCrntSegment, FID_DecideCrntSegment, FID_DecideCrntSegment },
    { Qt::CTRL+Qt::Key_O, FID_None, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::CTRL+Qt::Key_P, FID_None, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::CTRL+Qt::Key_S, FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_ShortenSegment },   
    { Qt::CTRL+Qt::Key_U, FID_None, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::CTRL+Qt::Key_At, FID_None, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu },
    { Qt::Key_Home,      FID_None, FID_CaretToFirst, FID_None, FID_None, FID_None },
    { Qt::Key_End,       FID_None, FID_CaretToLast, FID_None, FID_None, FID_None },
    { Qt::Key_Left,      FID_None, FID_CaretBackward, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::Key_Right,     FID_None, FID_CaretForward, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::Key_Up,        FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_None },
    { Qt::Key_Down,      FID_None, FID_DecideAllSegments, FID_DecideCrntSegment, FID_DecideCrntSegment, FID_DecideCrntSegment },
    { Qt::CTRL+Qt::Key_Left,   FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_ToFirstSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_Right,  FID_None, FID_CaretToLast, FID_ToLastSegment, FID_ToLastSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_Up,     FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_None },
    { Qt::SHIFT+Qt::Key_Left,  FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_ShortenSegment },    
    { Qt::SHIFT+Qt::Key_Right, FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_LengthenSegment },
    { Qt::SHIFT+Qt::Key_Up,    FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_None },
    { Qt::SHIFT+Qt::Key_Down,  FID_None, FID_None, FID_DecideCrntSegment, FID_DecideCrntSegment, FID_DecideCrntSegment },
    { Qt::Key_PageUp,    FID_None, FID_None, FID_PreviousCandidateGroup, FID_PreviousCandidateGroup, FID_None },
    { Qt::Key_PageDown,  FID_None, FID_None, FID_NextCandidateGroup, FID_NextCandidateGroup, FID_None }, 
    { Qt::Key_F6,        FID_None, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::Key_F7,        FID_None, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::Key_F8,        FID_None, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::Key_F9,        FID_None, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::Key_F10,       FID_None, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu },
    { Qt::Key_Henkan,    FID_ReconvertClipboardString, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },  
    { Qt::CTRL+Qt::Key_F7,  FID_ExecDictTool, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F12, FID_ShowPropertyDialog, FID_None, FID_None, FID_None, FID_None }, },
  
  { { Qt::Key_Space,      FID_InsertSpace, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },  // KINPUT2
    { Qt::CTRL+Qt::Key_Space, FID_None, FID_ConvertToOneSegment, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::SHIFT+Qt::Key_Space, FID_InsertOtherWidthSpace, FID_ConvertAllSegments, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment },
    { Qt::Key_Return,     FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Enter,      FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::SHIFT+Qt::Key_Return, FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::SHIFT+Qt::Key_Enter,  FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Backspace,  FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::Key_Delete,     FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_A, FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_ToFirstSegment, FID_ToFirstSegment },
    { Qt::CTRL+Qt::Key_B, FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_BackwardSegment },
    { Qt::CTRL+Qt::Key_D, FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_E, FID_None, FID_CaretToLast, FID_ToLastSegment, FID_ToLastSegment, FID_ToLastSegment },
    { Qt::CTRL+Qt::Key_F, FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::CTRL+Qt::Key_G, FID_None, FID_DeleteAll, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::CTRL+Qt::Key_H, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::CTRL+Qt::Key_I, FID_None, FID_None, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_M, FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_N, FID_None, FID_None, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::SHIFT+Qt::Key_N, FID_None, FID_None, FID_NextCandidateGroup, FID_NextCandidateGroup, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::Key_O, FID_None, FID_None, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_P, FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::SHIFT+Qt::Key_P, FID_None, FID_None, FID_PreviousCandidateGroup, FID_PreviousCandidateGroup, FID_ConvertCrntSegment },
    { Qt::Key_Left,      FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_BackwardSegment },
    { Qt::Key_Right,     FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::Key_Up,        FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment },
    { Qt::Key_Down,      FID_None, FID_None, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::Key_Left,  FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_ToFirstSegment, FID_ToFirstSegment },
    { Qt::CTRL+Qt::Key_Right, FID_None, FID_CaretToLast, FID_ToLastSegment, FID_ToLastSegment, FID_ToLastSegment },
    { Qt::CTRL+Qt::Key_Up,    FID_None, FID_None, FID_PreviousCandidateGroup, FID_PreviousCandidateGroup, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::Key_Down,  FID_None, FID_None, FID_NextCandidateGroup, FID_NextCandidateGroup, FID_ConvertCrntSegment },
    { Qt::SHIFT+Qt::Key_Left,  FID_None, FID_None, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::SHIFT+Qt::Key_Right, FID_None, FID_None, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::SHIFT+Qt::Key_Up,    FID_None, FID_None, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment },
    { Qt::SHIFT+Qt::Key_Down,  FID_None, FID_None, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::Key_Home,      FID_None, FID_None, FID_FirstCandidate, FID_FirstCandidate, FID_ConvertCrntSegment },
    { Qt::Key_End,       FID_None, FID_None, FID_LastCandidate, FID_LastCandidate, FID_ConvertCrntSegment },
    { Qt::Key_PageUp,    FID_None, FID_None, FID_PreviousCandidateGroup, FID_PreviousCandidateGroup, FID_ConvertCrntSegment },
    { Qt::Key_PageDown,  FID_None, FID_None, FID_NextCandidateGroup, FID_NextCandidateGroup, FID_ConvertCrntSegment },
    { Qt::Key_F5,        FID_SwitchInputMethod, FID_None, FID_None, FID_None, FID_None },
    { Qt::Key_F6,        FID_SetToHiraganaMode, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::Key_F7,        FID_SetToKatakanaMode, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::Key_F8,        FID_SetToHankakuKanaMode, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::Key_F9,        FID_SetToZenkakuEisuMode, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::Key_F10,       FID_None, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu },
    { Qt::Key_F11,       FID_ExecDictTool, FID_None, FID_None, FID_None, FID_None },
    { Qt::Key_F12,       FID_ShowPropertyDialog, FID_None, FID_None, FID_None, FID_None },
    { Qt::Key_Henkan,    FID_ReconvertClipboardString, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },  },
  
  { { Qt::Key_Space,     FID_InsertSpace, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },    // VJE
    { Qt::CTRL+Qt::Key_Space, FID_None, FID_ConvertToOneSegment, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::SHIFT+Qt::Key_Space, FID_InsertOtherWidthSpace, FID_ConvertAllSegments, FID_LengthenSegment, FID_PreviousCandidate, FID_LengthenSegment },
    { Qt::CTRL+Qt::SHIFT+Qt::Key_Space, FID_InsertMultibyteSpace, FID_None, FID_BackwardSegment, FID_BackwardSegment, FID_None },
    { Qt::Key_Return,    FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::Key_Enter,     FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_Return,    FID_None, FID_DecideAllSegments, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_Enter,     FID_None, FID_DecideAllSegments, FID_None, FID_None, FID_None },
    { Qt::SHIFT+Qt::Key_Return,    FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_ConvertCrntSegment },
    { Qt::SHIFT+Qt::Key_Enter,     FID_None, FID_DecidePredictedCandidate, FID_DecideAllSegments, FID_DecideAllSegments, FID_ConvertCrntSegment },
    { Qt::Key_Backspace, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_None },
    { Qt::Key_Delete,    FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::Key_Escape,    FID_None, FID_DeleteAll, FID_CancelConversion, FID_CancelConversion, FID_CancelConversion },
    { Qt::SHIFT+Qt::Key_Escape, FID_None, FID_DeleteAll, FID_DeleteAll, FID_CancelConversion, FID_DeleteAll },
    { Qt::CTRL+Qt::Key_A, FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_C, FID_None, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::Key_D, FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::CTRL+Qt::Key_E, FID_None, FID_CaretToFirst, FID_PreviousCandidate, FID_PreviousCandidateGroup, FID_None },
    { Qt::CTRL+Qt::Key_F, FID_None, FID_CaretToLast, FID_ToLastSegment, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_G, FID_None, FID_DeleteForwardChar, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_H, FID_None, FID_DeleteBackwardChar, FID_CancelConversion, FID_CancelConversion, FID_None },
    { Qt::CTRL+Qt::Key_I, FID_None, FID_None, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::CTRL+Qt::Key_J, FID_None, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::CTRL+Qt::Key_K, FID_None, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::CTRL+Qt::Key_L, FID_None, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::CTRL+Qt::Key_M, FID_None, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments, FID_DecideAllSegments },
    { Qt::CTRL+Qt::Key_O, FID_None, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::CTRL+Qt::Key_Q, FID_None, FID_CaretBackward, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_R, FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_PreviousCandidate, FID_BackwardSegment },
    { Qt::CTRL+Qt::Key_S, FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_BackwardSegment },
    { Qt::CTRL+Qt::Key_W, FID_None, FID_CaretForward, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },
    { Qt::CTRL+Qt::Key_X, FID_None, FID_CaretToLast, FID_NextCandidate, FID_NextCandidateGroup, FID_None },
    { Qt::CTRL+Qt::Key_Z, FID_None, FID_ConvertAllSegments, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment },
    { Qt::Key_Home,      FID_None, FID_CaretToFirst, FID_ToFirstSegment, FID_None, FID_None },
    { Qt::Key_End,       FID_None, FID_CaretToLast, FID_ToLastSegment, FID_None, FID_None },
    { Qt::Key_Left,      FID_None, FID_CaretBackward, FID_BackwardSegment, FID_BackwardSegment, FID_BackwardSegment },
    { Qt::Key_Right,     FID_None, FID_CaretForward, FID_ForwardSegment, FID_ForwardSegment, FID_ForwardSegment },
    { Qt::Key_Up,        FID_None, FID_ConvertAllSegments, FID_PreviousCandidate, FID_PreviousCandidate, FID_ConvertCrntSegment },
    { Qt::Key_Down,      FID_None, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::Key_Up,     FID_None, FID_ConvertAllSegments, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::CTRL+Qt::Key_Down,   FID_None, FID_ConvertAllSegments, FID_DecideCrntSegment, FID_DecideCrntSegment, FID_LengthenSegment },
    { Qt::SHIFT+Qt::Key_Left,  FID_None, FID_CaretBackward, FID_ShortenSegment, FID_ShortenSegment, FID_ShortenSegment },
    { Qt::SHIFT+Qt::Key_Right, FID_None, FID_CaretForward, FID_LengthenSegment, FID_LengthenSegment, FID_LengthenSegment },    
    { Qt::SHIFT+Qt::Key_Up,    FID_None, FID_ConvertAllSegments, FID_PreviousCandidate, FID_PreviousCandidateGroup, FID_ConvertCrntSegment },
    { Qt::SHIFT+Qt::Key_Down,  FID_None, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidateGroup, FID_ConvertCrntSegment },
    { Qt::Key_PageUp,    FID_None, FID_CaretToFirst, FID_PreviousCandidate, FID_PreviousCandidateGroup, FID_None },
    { Qt::Key_PageDown,  FID_None, FID_CaretToLast, FID_NextCandidate, FID_NextCandidateGroup, FID_None }, 
    { Qt::Key_F6,        FID_None, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira, FID_ConvertToHira },
    { Qt::Key_F7,        FID_None, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana, FID_ConvertToKana },
    { Qt::Key_F8,        FID_None, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu, FID_ConvertToZenkakuEisu },
    { Qt::Key_F9,        FID_None, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana, FID_ConvertToHankakuKana },
    { Qt::Key_F10,       FID_None, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu, FID_ConvertToHankakuEisu },
    { Qt::Key_Henkan,    FID_ReconvertClipboardString, FID_ConvertAllSegments, FID_NextCandidate, FID_NextCandidate, FID_ConvertCrntSegment },
    { Qt::CTRL+Qt::Key_F2,  FID_ExecDictTool, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F3,  FID_SwitchInputMethod, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F6,  FID_SetToHiraganaMode, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F7,  FID_SetToKatakanaMode, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F8,  FID_SetToZenkakuEisuMode, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F9,  FID_SetToHankakuKanaMode, FID_None, FID_None, FID_None, FID_None },
    { Qt::CTRL+Qt::Key_F10, FID_ShowPropertyDialog, FID_None, FID_None, FID_None, FID_None }, },

  { { 0 } },     // Current setting  (dummy)
};


void KeyAssigner::init()
{
  FuncName[FID_None]                     = "-";
  FuncName[FID_InsertChar]               = tr("入力");
  FuncName[FID_NextCandidate]            = tr("次候補");
  FuncName[FID_NextCandidateGroup]       = tr("次候補群");
  FuncName[FID_PreviousCandidate]        = tr("前候補");
  FuncName[FID_PreviousCandidateGroup]   = tr("前候補群");
  FuncName[FID_FirstCandidate]           = tr("候補先頭");
  FuncName[FID_LastCandidate]            = tr("候補末尾");
  FuncName[FID_ConvertAllSegments]       = tr("全変換");
  FuncName[FID_ConvertCrntSegment]       = tr("文節変換");
  FuncName[FID_ConvertToOneSegment]      = tr("単文節変換");
  FuncName[FID_DeleteForwardChar]        = tr("１字削除");
  FuncName[FID_DeleteBackwardChar]       = tr("前字削除");
  FuncName[FID_DeleteAll]                = tr("全削除");
  FuncName[FID_DecideAllSegments]        = tr("全確定");
  FuncName[FID_DecideCrntSegment]        = tr("文節確定");
  FuncName[FID_DecidePredictedCandidate] = tr("予測候補確定");
  FuncName[FID_CancelConversion]         = tr("全戻し");
  FuncName[FID_LengthenSegment]          = tr("文節長＋１");
  FuncName[FID_ShortenSegment]           = tr("文節長−１");
  FuncName[FID_InsertSpace]              = tr("空白文字");
  FuncName[FID_InsertAsciiSpace]         = tr("半角空白");
  FuncName[FID_InsertMultibyteSpace]     = tr("全角空白");
  FuncName[FID_InsertOtherWidthSpace]    = tr("別幅空白");
  FuncName[FID_ConvertToHira]            = tr("ひらがな");
  FuncName[FID_ConvertToKana]            = tr("カタカナ");
  FuncName[FID_ConvertToHankakuKana]     = tr("半角カナ");
  FuncName[FID_ConvertToHankakuEisu]     = tr("半角英数");
  FuncName[FID_ConvertToZenkakuEisu]     = tr("全角英数");
  FuncName[FID_ForwardSegment]           = tr("文節右移動");
  FuncName[FID_BackwardSegment]          = tr("文節左移動");
  FuncName[FID_ToFirstSegment]           = tr("文節先頭");
  FuncName[FID_ToLastSegment]            = tr("文節末尾");
  FuncName[FID_CaretForward]             = tr("右移動");
  FuncName[FID_CaretBackward]            = tr("左移動");
  FuncName[FID_CaretToFirst]             = tr("先頭移動");
  FuncName[FID_CaretToLast]              = tr("末尾移動");
  FuncName[FID_SwitchZenkakuEisuMode]    = tr("全角英数切替");
  FuncName[FID_SetToHiraganaMode]        = tr("ひらがなモード");
  FuncName[FID_SetToKatakanaMode]        = tr("カタカナモード");
  FuncName[FID_SetToHankakuKanaMode]     = tr("半角カナモード");
  FuncName[FID_SetToZenkakuEisuMode]     = tr("全角英数モード");
  FuncName[FID_SwitchInputMethod]        = tr("入力方式切替");
  FuncName[FID_ShowPropertyDialog]       = tr("プロパティ");
  FuncName[FID_ExecDictTool]             = tr("辞書ツール");
  FuncName[FID_ReconvertClipboardString] = tr("再変換(実験的)");

  _keyassigntbl->setNumCols( NUM_OF_CONVSTATUS + 1 );
  for (int i = 0; i < _keyassigntbl->numCols(); ++i)
    _keyassigntbl->setColumnStretchable(i, TRUE);

  _cmbloadsetting->insertItem(tr("MS-IME風"), ST_MSIME);
  _cmbloadsetting->insertItem(tr("ATOK風"), ST_ATOK);
  _cmbloadsetting->insertItem(tr("Kinput2風"), ST_KINPUT2);
  _cmbloadsetting->insertItem(tr("VJE風"), ST_VJE);
  QToolTip::add(_cmbloadsetting, tr("リストから選択されたキー設定を読み込みます。"));
}


void KeyAssigner::polish()
{
  loadSetting(ST_CURRENT_SETTING);
  QDialog::polish();
}


void KeyAssigner::saveSetting()
{
  for (int row = 0; row < MAX_AVAILABLE_KEYS; row++) {
    if (row < _keyassigntbl->numRows()) {
      QKeySequence keyseq( _keyassigntbl->text(row, 0) );
      int key = keyseq[0] & ~UNICODE_ACCEL;
      Config::writeEntry(PREFIX + QString::number(row) + "-" + QString::number(0), key);

      for (int col = 1; col <= NUM_OF_CONVSTATUS; col++) {
	Config::writeEntry(PREFIX + QString::number(row) 
			   + "-" + QString::number(col), (int)functionID(_keyassigntbl->text(row, col)));
      }
    
    } else {
      // 0 clear
      for (int col = 0; col <= NUM_OF_CONVSTATUS; col++)
	Config::writeEntry(PREFIX + QString::number(row) 
			   + "-" + QString::number(col), (int)0);
    }
  }

  Config::writeEntry(_cmbloadsetting->name(), _cmbloadsetting->currentItem());
  KeyAssign[ST_CURRENT_SETTING][0][0] = 0;  // implies reloading function
}


void KeyAssigner::loadSetting( int index )
{
  _keyassigntbl->setNumRows(MAX_AVAILABLE_KEYS);

  int row = 0;
  switch ( index ) {
  case ST_CURRENT_SETTING:
    for (row = 0; row < MAX_AVAILABLE_KEYS; row++) {
      bool ok;
      int key = Config::readNumEntry(PREFIX + QString::number(row) + "-" + QString::number(0), 0, &ok);
      if ( !ok || !key )
	break;

      QKeySequence keyseq(key);
      _keyassigntbl->insertRows(row);
      _keyassigntbl->setText(row, 0, (QString)keyseq);
      
      for (int col = 1; col <= NUM_OF_CONVSTATUS; col++) {
	int funcid = Config::readNumEntry(PREFIX + QString::number(row) + "-" + QString::number(col), FID_None);
	_keyassigntbl->setText(row, col, functionName( (FuncID)funcid ));
      }
    }
    _keyassigntbl->setNumRows(row);
    _cmbloadsetting->setCurrentItem( Config::readNumEntry(_cmbloadsetting->name(), 0) );
    break;
    
  case ST_MSIME:
  case ST_ATOK:
  case ST_KINPUT2:
  case ST_VJE:
    for (row = 0; row < MAX_AVAILABLE_KEYS; row++) {
      if ( ! KeyAssign[index][row][0] )
	break;

      QKeySequence keyseq( KeyAssign[index][row][0] );
      _keyassigntbl->insertRows(row);
      _keyassigntbl->setText(row, 0, keyseq);
      
      for (int col = 1; col <= NUM_OF_CONVSTATUS; col++) {
	_keyassigntbl->setText(row, col, functionName( (FuncID)KeyAssign[index][row][col] ));
      }
    }
    _keyassigntbl->setNumRows(row);
    break;
    
  default:
    break;
  }
}


QString KeyAssigner::functionName( FuncID id )
{
  bool ok;
  QString str = FuncName.at(id, &ok);
  return ok ? str : QString::null;
}


FuncID KeyAssigner::functionID( const QString & func )
{
  int id;
  for (id = 0; id < FID_NumFunctions; ++id) {
    if (FuncName[id] == func) 
      break;
  }
  return (id < FID_NumFunctions) ? (FuncID)id : FID_None;
}


FuncID KeyAssigner::functionID( int key, ConvStatus stat )
{
  const int crntset = ST_CURRENT_SETTING;
  if ( !KeyAssign[crntset][0][0] ) {
    // Loads data
    for (int i = 0; i < MAX_AVAILABLE_KEYS; ++i) {
      KeyAssign[crntset][i][0] = Config::readNumEntry(PREFIX + QString::number(i) + "-" + QString::number(0), 0);
      for (int j = 1; j <= NUM_OF_CONVSTATUS; ++j) {
	KeyAssign[crntset][i][j] = Config::readNumEntry(PREFIX + QString::number(i) + "-" + QString::number(j), FID_None);
      }
    }
  }

  for (int i = 0; i < MAX_AVAILABLE_KEYS; ++i) {
    if (KeyAssign[crntset][i][0] == key) {
      return (FuncID)KeyAssign[crntset][i][(int)stat];
    }
  }
  
  if (key & ~(0xff | SHIFT)) {
    return FID_None;
  }
  
  return FID_InsertChar;
}


void KeyAssigner::accept()
{
  saveSetting();
  QDialog::accept();
}


void KeyAssigner::reject()
{
  QDialog::reject();
  loadSetting(ST_CURRENT_SETTING);
}


void KeyAssigner::saveDefaultSetting()
{
  //const int setting = ST_KINPUT2;
  const int setting = ST_MSIME;
  for (int row = 0; row < MAX_AVAILABLE_KEYS; row++) {
    if ( KeyAssign[setting][row][0] ) {
      for (int col = 0; col <= NUM_OF_CONVSTATUS; col++) {
	Config::writeEntry(PREFIX + QString::number(row) + "-" + QString::number(col), KeyAssign[setting][row][col], FALSE);
      }

    } else {
      for (int col = 0; col <= NUM_OF_CONVSTATUS; col++) {
	Config::writeEntry(PREFIX + QString::number(row) + "-" + QString::number(col), (int)0, FALSE);
      }
    }
  }

  Config::writeEntry("_cmbloadsetting", setting, FALSE);
}
