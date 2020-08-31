//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2020 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// This program demonstrates iterating over the vertices of
// a polyline

#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

#include <Windows.h>
#include <stdlib.h>
#include <rxobject.h>
#include <rxregsvc.h>
#include <aced.h>
#include <dbents.h>
#include <adslib.h>
#include <geassign.h>
#include <dbapserv.h>
////#include <dbmain.h>
#include "tchar.h"



#define ET_NORM 1 // Normal entity  
#define ET_TBL  2 // Table  
#define ET_VPORT  3 // Table numbers  
#define ET_LTYPE  4 
#define ET_LAYER  5 
#define ET_STYLE  6 
#define ET_VIEW   7 
#define ET_UCS    8 
#define ET_BLOCK  9 
// Get basic C-language type from AutoCAD DXF group code (RTREAL,
// RTANG are doubles, RTPOINT double[2], RT3DPOINT double[3], 
// RTENAME long[2]). The etype argument is one of the ET_
// definitions. 
//
// Returns RTNONE if grpcode isn't one of the known group codes. 
// Also, sets "inxdata" argument to TRUE if DXF group is in XDATA.  
//
short dxftype(short grpcode, short etype, int* inxdata)
{
    short rbtype = RTNONE;
    *inxdata = FALSE;
    if (grpcode >= 1000) {  // Extended data (XDATA) groups
        *inxdata = TRUE;
        if (grpcode == 1071)
            rbtype = RTLONG; // Special XDATA case  
        else
            grpcode %= 1000; // All other XDATA groups match.  
    } // regular DXF code ranges  
    if (grpcode <= 49) {
        if (grpcode >= 20) // 20 to 49  
            rbtype = RTREAL;
        else if (grpcode >= 10) { // 10 to 19  
            if (etype == ET_VIEW) // Special table cases
                rbtype = RTPOINT;
            else if (etype == ET_VPORT && grpcode <= 15)
                rbtype = RTPOINT;
            else // Normal point  
                rbtype = RT3DPOINT; // 10: start point, 11: endpoint
        }
        else if (grpcode >= 0) // 0 to 9  
            rbtype = RTSTR; // Group 1004 in XDATA is binary
        else if (grpcode >= -2)
            // -1 = start of normal entity -2 = sequence end, etc.  
            rbtype = RTENAME;
        else if (grpcode == -3)
            rbtype = RTSHORT; // Extended data (XDATA) sentinel 
    }
    else {
        if (grpcode <= 59) // 50 to 59  
            rbtype = RTANG; // double  
        else if (grpcode <= 79) // 60 to 79  
            rbtype = RTSHORT;
        else if (grpcode < 210)
            ;
        else if (grpcode <= 239) // 210 to 239  
            rbtype = RT3DPOINT;
        else if (grpcode == 999) // Comment  
            rbtype = RTSTR;
    }
    return rbtype;
}


void listPline();
void iterate(AcDbObjectId id);
void initApp();
void unloadApp();
extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode, void*);

// This is the main function of this app.  It allows the
// user to select an entity.  It then checks to see if the
// entity is a 2d-polyline.  If so, then it calls iterate
// passing in the objectId of the pline.
// 
void listPline()
{
    int rc;
    ads_name en;
    AcGePoint3d pt;
    rc = acedEntSel(_T("\nSelect a polyline: "), en,
        asDblArray(pt));

    if (rc != RTNORM) {
        acutPrintf(_T("\nError during object selection"));
        return;
    }

    AcDbObjectId eId;
    acdbGetObjectId(eId, en);

    AcDbObject *pObj;
    acdbOpenObject(pObj, eId, AcDb::kForRead);
    if (pObj->isKindOf(AcDb2dPolyline::desc())) {
        pObj->close();
        iterate(eId);
    } else {
        pObj->close();
        acutPrintf(_T("\nSelected entity is not an AcDb2dPolyline. \nMake sure the setvar PLINETYPE is set to 0 before createing a polyline"));
    }
}


// Accepts the object ID of an AcDb2dPolyline, opens it, and gets
// a vertex iterator. It then iterates through the vertices,
// printing out the vertex location.
// 
void iterate(AcDbObjectId plineId)
{
    AcDb2dPolyline *pPline;
    acdbOpenObject(pPline, plineId, AcDb::kForRead);

    AcDbObjectIterator *pVertIter= pPline->vertexIterator();
    pPline->close();  // Finished with the pline header.

    AcDb2dVertex *pVertex;
    AcGePoint3d location;
    AcDbObjectId vertexObjId;
    for (int vertexNumber = 0; !pVertIter->done();
        vertexNumber++, pVertIter->step())
    {
        vertexObjId = pVertIter->objectId();
        acdbOpenObject(pVertex, vertexObjId,
            AcDb::kForRead);
        location = pVertex->position();
        pVertex->close();

        acutPrintf(_T("\nVertex #%d's location is")
            _T(" : %0.3f, %0.3f, %0.3f"), vertexNumber,
            location[X], location[Y], location[Z]);
    }
    delete pVertIter;
}


// Initialization function called from acrxEntryPoint during
// kInitAppMsg case.  This function is used to add commands
// to the command stack.
// 
void initApp()
{
    acedRegCmds->addCommand(
        _T("ASDK_PLINETEST_COMMANDS"),
        _T("ASDK_ITERATE"), 
        _T("ITERATE"), 
        ACRX_CMD_MODAL,
        listPline
    );

    acutPrintf(_T("\nHello World6.\n"));
    //listPline();

    // inspect the block definition named "remediationWellWithNoConstituentsOfConcernInPerchedGroundwater"
    // to figure out how dynamic paramters and actions are represented
    AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
    AcDbBlockTable* pBlockTable;
    AcDbBlockTableRecord* pBlockTableRecord;
    pDb->getSymbolTable(pBlockTable, AcDb::kForRead);
    pBlockTable->getAt(_T("remediationWellWithNoConstituentsOfConcernInPerchedGroundwater"), pBlockTableRecord, AcDb::kForRead);
    pBlockTable->close();
    //insepct any xdata that the block table record  might own:
    resbuf* pResbuf;
    pResbuf = pBlockTableRecord->xData();
    if (pResbuf == NULL) {
        acutPrintf(_T("%s"), _T("block table record has no xdata\n"));
    }
    else {
        acutPrintf(_T("%s"), _T("block table record has some xdata\n"));
        while (pResbuf != NULL) {
            int inxdata;
            short theType;
            std::wstring typeName;
            theType = dxftype(pResbuf->restype, ET_NORM, &inxdata);
            std::wstring message;
            message += _T("Found an xdata item\n");


            switch (theType) {
                case RTNONE:  typeName = _T("RTNONE");         break;
                case RTREAL:  typeName = _T("RTREAL");         break;
                case RTPOINT:  typeName = _T("RTPOINT");       break;
                case RTSHORT:  typeName = _T("RTSHORT");       break;
                case RTANG:  typeName = _T("RTANG");           break;
                case RTSTR:  typeName = _T("RTSTR");           break;
                case RTENAME:  typeName = _T("RTENAME");       break;
                case RTPICKS:  typeName = _T("RTPICKS");       break;
                case RTORINT:  typeName = _T("RTORINT");       break;
                case RT3DPOINT:  typeName = _T("RT3DPOINT");   break;
                case RTLONG:  typeName = _T("RTLONG");         break;
                case RTVOID:  typeName = _T("RTVOID");         break;
                case RTLB:  typeName = _T("RTLB");             break;
                case RTLE:  typeName = _T("RTLE");             break;
                case RTDOTE:  typeName = _T("RTDOTE");         break;
                case RTNIL:  typeName = _T("RTNIL");           break;
                case RTDXF0:  typeName = _T("RTDXF0");         break;
                case RTT:  typeName = _T("RTT");               break;
                case RTRESBUF:  typeName = _T("RTRESBUF");     break;
                case RTMODELESS:  typeName = _T("RTMODELESS"); break;
                default: break;
            }

            message += _T("\ttype: ") + typeName + _T("\n");
            message += _T("\tvalue: ");
            switch (theType) {
            case RTNONE:  
                message += L"(none)";
                break;
            case RTREAL:
            case RTANG:
                message += std::to_wstring(pResbuf->resval.rreal );         
                break;
            case RTPOINT: 
            case RT3DPOINT:
                message += std::to_wstring(pResbuf->resval.rpoint[0]);
                message += L", ";
                message += std::to_wstring(pResbuf->resval.rpoint[1]);
                message += L", ";
                message += std::to_wstring(pResbuf->resval.rpoint[2]);
                break;
            case RTSHORT: 
            case RTORINT:
                message += std::to_wstring(pResbuf->resval.rint);
                break;
            case RTSTR:  
                message += pResbuf->resval.rstring;
                break;
            case RTENAME:
            case RTPICKS:
                message += std::to_wstring(pResbuf->resval.rlname[0]);
                message += L" ";
                message += std::to_wstring(pResbuf->resval.rlname[1]);
                break;
            case RTLONG: 
                message += std::to_wstring(pResbuf->resval.rlong);
                break;
            case RTVOID:  
                message += L"<void>";
                break;
            case RTLB:  
                message += L"<list begin>";
                break;
            case RTLE:
                message += L"<list end>";
                break;
            case RTDOTE: 
                message += L"<dot>";
                break;
            case RTNIL: 
                message += L"<nil>";
                break;
            case RTDXF0: 
                message += L"<dxf0>";
                break;
            case RTT: 
                message += L"<t>";
                break;
            case RTRESBUF: 
                message += L"another resbuf";
                break;
            case RTMODELESS:  
                break;
            default: break;
            }
            message += L"\n";
            acutPrintf(message.c_str());
            pResbuf = pResbuf->rbnext;
        }


    };
    acutRelRb(pResbuf);

    
    AcDbBlockTableRecordIterator* pBlockTableRecordIterator;
    pBlockTableRecord->newIterator(pBlockTableRecordIterator);
    AcDbEntity* pEntity;
    for (pBlockTableRecordIterator->start(); !pBlockTableRecordIterator->done(); pBlockTableRecordIterator->step()) {
        pBlockTableRecordIterator->getEntity(pEntity, AcDb::kForRead);
        AcDbHandle handle;
        ACHAR sHandle[17];
        lstrcpy(sHandle, _T(""));
        pEntity->getAcDbHandle(handle);
        handle.getIntoAsciiBuffer(sHandle, (size_t)17);
        acutPrintf(
            _T("classname: %s, handle: %s\n"), 
            pEntity->isA()->name(),
            sHandle
        );
        pEntity->close();
        
    }
    pBlockTableRecord->close();
    delete pBlockTableRecordIterator;

}

// Clean up function called from acrxEntryPoint during the
// kUnloadAppMsg case.  This function removes this apps
// command set from the command stack.
// 
void unloadApp()
{
    acedRegCmds->removeGroup(_T("ASDK_PLINETEST_COMMANDS"));
    acutPrintf(_T("\nGoodbye.\n"));
}

AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* packet)
{
    switch (msg) {
        case AcRx::kInitAppMsg:
            void* appId;
            appId = packet;

            // make this application unloadable:
            acrxDynamicLinker->unlockApplication(appId);

		    acrxDynamicLinker->registerAppMDIAware(appId);
            initApp();
            break;
        case AcRx::kUnloadAppMsg:
            unloadApp();
    }
    return AcRx::kRetOK;
}
