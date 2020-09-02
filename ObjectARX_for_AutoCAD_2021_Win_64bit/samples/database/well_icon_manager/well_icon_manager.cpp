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
#include <dbmain.h>
#include <dbeval.h>
#include "tchar.h"
#include <list>
#include <rxclass.h>
#include <rxmember.h>




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

class ResbufWrapper {
    private:
       resbuf* pResbuf;

    public:
        ResbufWrapper(resbuf* pResbuf) {
            this->pResbuf = pResbuf;
        }

        ~ResbufWrapper() {
            acutRelRb(pResbuf);
        }

        static std::wstring resultTypeCodeToString(short resultTypeCode) {
            std::wstring returnValue;
            switch(resultTypeCode){
                case RTNONE       : returnValue = L"RTNONE";      break;
                case RTREAL       : returnValue = L"RTREAL";      break;
                case RTPOINT      : returnValue = L"RTPOINT";     break;
                case RTSHORT      : returnValue = L"RTSHORT";     break;
                case RTANG        : returnValue = L"RTANG";       break;
                case RTSTR        : returnValue = L"RTSTR";       break;
                case RTENAME      : returnValue = L"RTENAME";     break;
                case RTPICKS      : returnValue = L"RTPICKS";     break;
                case RTORINT      : returnValue = L"RTORINT";     break;
                case RT3DPOINT    : returnValue = L"RT3DPOINT";   break;
                case RTLONG       : returnValue = L"RTLONG";      break;
                case RTVOID       : returnValue = L"RTVOID";      break;
                case RTLB         : returnValue = L"RTLB";        break;
                case RTLE         : returnValue = L"RTLE";        break;
                case RTDOTE       : returnValue = L"RTDOTE";      break;
                case RTNIL        : returnValue = L"RTNIL";       break;
                case RTDXF0       : returnValue = L"RTDXF0";      break;
                case RTT          : returnValue = L"RTT";         break;
                case RTRESBUF     : returnValue = L"RTRESBUF";    break;
                case RTMODELESS   : returnValue = L"RTMODELESS";  break;
                default           : returnValue = std::wstring(L"UNKNOWN_RETURN_TYPE_CODE:") + std::to_wstring(resultTypeCode);
            };
            return returnValue;
        }

        std::wstring toString() {
            std::wstring returnValue;
            returnValue += L"resbuf:\n";
            if (pResbuf == NULL) {
                returnValue += std::wstring(_T("\t")) + _T("NULL");
            } else {
                for (resbuf* head = pResbuf; head != NULL; head = head->rbnext) {
                    int inxdata;
                    short resultTypeCode;
                    resultTypeCode = dxftype(head->restype, ET_NORM, &inxdata);
                    returnValue += std::wstring(_T("\t")) + _T("type: ") + ResbufWrapper::resultTypeCodeToString(resultTypeCode) + L", ";
                    returnValue += std::wstring(_T("value: "));

                    switch (resultTypeCode) {
                    case RTNONE:
                        returnValue += L"(none)";
                        break;
                    case RTREAL:
                    case RTANG:
                        returnValue += std::to_wstring(head->resval.rreal);
                        break;
                    case RTPOINT:
                    case RT3DPOINT:
                        returnValue += 
                           std::to_wstring(head->resval.rpoint[0])
                           + L", "
                           + std::to_wstring(head->resval.rpoint[1])
                           + L", "
                           + std::to_wstring(head->resval.rpoint[2])
                           + L"";
                        break;
                    case RTSHORT:
                    case RTORINT:
                        returnValue += std::to_wstring(head->resval.rint);
                        break;
                    case RTSTR:
                        returnValue += head->resval.rstring;
                        break;
                    case RTENAME:
                    case RTPICKS:
                        returnValue += 
                            std::to_wstring(head->resval.rlname[0])
                            + L" "
                            + std::to_wstring(head->resval.rlname[1])
                            + L"";
                        break;
                    case RTLONG:
                        returnValue += std::to_wstring(head->resval.rlong);
                        break;
                    case RTVOID:
                        returnValue += L"<void>";
                        break;
                    case RTLB:
                        returnValue += L"<list begin>";
                        break;
                    case RTLE:
                        returnValue += L"<list end>";
                        break;
                    case RTDOTE:
                        returnValue += L"<dot>";
                        break;
                    case RTNIL:
                        returnValue += L"<nil>";
                        break;
                    case RTDXF0:
                        returnValue += L"<dxf0>";
                        break;
                    case RTT:
                        returnValue += L"<t>";
                        break;
                    case RTRESBUF:
                        returnValue += ResbufWrapper((resbuf*) head->resval.mnLongPtr).toString();
                        break;
                    case RTMODELESS:
                        returnValue += L"<rtmodeless>";
                        break;
                    default: 
                        returnValue += L"value of resbuf of unknown type.";
                        break;
                    }
                    returnValue += L"\n";
                }
            }
            return returnValue;
        }
};


std::wstring handleToString(AcDbHandle handle) {
    ACHAR sHandle[17];
    //lstrcpy(sHandle, _T(""));
    handle.getIntoAsciiBuffer(sHandle, (size_t)17);
    return std::wstring(sHandle);
}

std::wstring objectIdToString(AcDbObjectId objectId) {
    if (objectId == AcDbObjectId::kNull) {
        return L"NULL";
    }
    else {
        //to do : catch some exceptions that might arise here.
        return std::wstring(objectId.objectClass()->name()) + L" (" + handleToString(objectId.handle()) + L")";
    }
}

std::vector<AcRxClass*> getAncestry(AcRxClass *x) {
    std::vector<AcRxClass*> ancestry = std::vector<AcRxClass*>();
    AcRxClass* ancestor;
    ancestor = x;
    while (true) {
        ancestry.push_back(ancestor);
        if (ancestor == AcRxObject::desc()) { break; }
        ancestor = ancestor->myParent();
    }
    return ancestry;
}

std::wstring ancestryToString(std::vector<AcRxClass*> ancestry) {
    std::wstring returnValue;
    for (int i = 0; i < ancestry.size();  i++) {
        returnValue += std::wstring(ancestry.at(i)->name()) + (i < ancestry.size() - 1 ? L", ": L"");
    }
    return returnValue;
}

std::vector<AcRxClass*> getAncestry(const AcRxObject * const x) {
    return getAncestry(x->isA());
}



void myAcutPrint(std::wstring x) {
    acutPrintf(x.c_str());
}

void myAcutPrintLine(std::wstring x, int tabLevel = 0) {
    myAcutPrint(std::wstring(tabLevel, L'\t') + x + L"\n");
}

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
    pBlockTable->getAt(_T("injectionWellWithNoConstituentsOfConcernInPerchedGroundwater"), pBlockTableRecord, AcDb::kForRead);
    pBlockTable->close();
    //inspect any xdata that the block table record  might own:
    acutPrintf((std::wstring(L"xData attached to the block table record: ") + ResbufWrapper(pBlockTableRecord->xData()).toString() + L"\n").c_str());

    int tabLevel = 0;

    //inspect any extension dictionary that the block table record might own:
    AcDbDictionary* pExtensionDictionary;
    if (pBlockTableRecord->extensionDictionary() == AcDbObjectId::kNull) {
        acutPrintf(_T("The block table record owns no extension dictionary.\n"));
    } else if (acdbOpenObject(pExtensionDictionary, pBlockTableRecord->extensionDictionary(), AcDb::kForRead) != Acad::eOk ) {
        acutPrintf(_T("Failed to open the block table's extension dictionary.\n"));
    } else {
        // in this case, the block table record has an extension dictionary (pExtensionDictionary), and we have opened it 
        myAcutPrintLine(
            std::wstring(L"The block table record (") 
            + objectIdToString(pBlockTableRecord->objectId()) 
            + L") has an extension dictionary " + objectIdToString(pBlockTableRecord->extensionDictionary()),
            //+ pExtensionDictionary->className() 
            //+ L" "
            //+ pExtensionDictionary->objectId().objectClass()->name()
            //+ L" "
            // + pBlockTableRecord->extensionDictionary().objectClass()->name(),
            tabLevel
        );
        tabLevel++;
        
        for (AcDbDictionaryIterator* pDictionaryIterator = pExtensionDictionary->newIterator();
            !pDictionaryIterator->done();
            pDictionaryIterator->next()
        )
        {
            std::wstring name = pDictionaryIterator->name();
            myAcutPrintLine(name + L": " + objectIdToString(pDictionaryIterator->objectId()), tabLevel);
            tabLevel++;
            AcDbObject* item;

            if (acdbOpenObject(item, pDictionaryIterator->objectId(), AcDb::kForRead) != Acad::eOk) 
            {
                myAcutPrintLine(L"unable to open the object.", tabLevel);
            }
            else if (name == std::wstring(L"ACAD_ENHANCEDBLOCK") && item->isKindOf( AcDbEvalGraph::desc())) 
            {
                    myAcutPrintLine(L"found an enhanced (aka dynamic ?) block.", tabLevel);
                    tabLevel++;
                    AcDbEvalGraph* evalGraphP = (AcDbEvalGraph*) item;
                    AcDbEvalNodeIdArray nodeIds;
                    Acad::ErrorStatus errorStatus = evalGraphP->getAllNodes(nodeIds);
                    if (errorStatus != Acad::ErrorStatus::eOk) 
                    {
                        myAcutPrintLine(L"encountered an error while attempting to get the nodes.", tabLevel);
                    }
                    else 
                    {
                        myAcutPrintLine(std::wstring(L"hooray we got the nodes.  There are ") + std::to_wstring(nodeIds.length()) + L" nodes.", tabLevel);
                        AcDbEvalEdgeInfoArray edges;
                        //AcDbEvalEdgeInfoArray incomingEdges;

                        //for (int i = 0; i < nodeIds.length(); i++) {
                        for (int i = nodeIds.length() - 1; i >= 0; i--) {

                            AcDbEvalNodeId nodeId = nodeIds.at(i);
                            AcDbObject* nodeP;
                            Acad::ErrorStatus errorStatus;
                            errorStatus = evalGraphP->getNode(nodeId, AcDb::kForRead, &nodeP);
                            if (errorStatus != Acad::eOk) {
                                myAcutPrintLine(std::wstring(L"failed to open node ") + std::to_wstring(i) + L", whose id is " + std::to_wstring(nodeId), tabLevel);
                            }
                            else {
                                myAcutPrintLine(std::wstring(L"succesfully opened node ") + std::to_wstring(i) 
                                    +L" (" + objectIdToString(nodeP->objectId()) + L")"
                                    + L", whose id is " + std::to_wstring(nodeId)
                                    + L" and whose class ancestry is "
                                    + ancestryToString(getAncestry(nodeP->isA())), 
                                    tabLevel
                                );  
                                evalGraphP->getOutgoingEdges(nodeId, edges);
                                //evalGraphP->getIncomingEdges(nodeId, incomingEdges);
                                //myAcutPrintLine(std::wstring(L"edges.length(): ") + std::to_wstring(edges.length()), tabLevel);
                                //myAcutPrintLine(std::wstring(L"incomingEdges.length(): ") + std::to_wstring(incomingEdges.length()), tabLevel);
                                acdbEntGet();
                                
                                
                                nodeP->close();
                            }      
                        }
                        myAcutPrintLine(std::wstring(L"edges:"), tabLevel);
                        tabLevel++;
                        for (int i = 0; i < edges.length(); i++)
                        {
                            AcDbObject* fromNode;
                            AcDbObject* toNode;
                            
                            evalGraphP->getNode(edges.at(i)->from(), AcDb::kForRead, &fromNode);
                            evalGraphP->getNode(edges.at(i)->to(), AcDb::kForRead, &toNode);

                            myAcutPrintLine(
                                std::to_wstring(edges.at(i)->from()) + L" (" + objectIdToString(fromNode->objectId()) + L")"
                                + L" --> " 
                                + std::to_wstring(edges.at(i)->to()) + L" (" + objectIdToString(toNode->objectId()) + L")"
                                + (edges.at(i)->isInvertible() ? L" invertible " : L"") 
                                + (edges.at(i)->isSuppressed() ? L" suppressed " : L"") 
                                , tabLevel
                            );
                            
                            fromNode->close();
                            toNode->close();
                        }
                        tabLevel--;
                        

                    }


                tabLevel--;
            } 
            else if (name == std::wstring(L"AcDbDynamicBlockRoundTripPurgePreventer") && std::wstring(item->isA()->name()) == std::wstring(L"AcDbDynamicBlockPurgePreventer")) 
            {
                myAcutPrintLine(L"found a purge preventer (what the hell is that?).", tabLevel);
                tabLevel++;

                myAcutPrintLine(std::wstring(L"ancestors of item->isA(): "), tabLevel);
                tabLevel++;
                std::list<AcRxClass*> ancestryList;

                AcRxClass* ancestor = item->isA();
                while (true) {
                    ancestryList.push_front(ancestor);
                    if (ancestor == AcRxObject::desc()) { break; }

                    ancestor = ancestor->myParent();
                }

                    
                for (decltype(ancestryList)::iterator it = ancestryList.begin(); it != ancestryList.end(); ++it) {
                    //myAcutPrintLine(std::wstring(L"ancestor ") + std::to_wstring(i) + L" class: " + (*it)->name() , tabLevel);
                    myAcutPrintLine((*it)->name(), tabLevel);
                }
                tabLevel--;

                if (false) {
                    //myAcutPrintLine(std::wstring(L"item->isA()->descendants()->isA()->name(): ") + ((AcRxObject*) item->isA()->descendants())->isA()->name(), tabLevel);
                    //myAcutPrintLine(std::wstring(L"item->isA()->descendants()->isA()->name(): ") + ((AcRxClass**) item->isA()->descendants())[0]->name(), tabLevel);
                    //myAcutPrintLine(std::wstring(L"item->isA()->descendants(): ") + std::to_wstring((uintptr_t) item->isA()->descendants()), tabLevel); //returns 0
                    //myAcutPrintLine(std::wstring(L"item->isA()->myParent()->descendants(): ") + std::to_wstring((uintptr_t)item->isA()->myParent()->descendants()), tabLevel); //returns 0
                    //myAcutPrintLine(std::wstring(L"item->isA()->myParent()->descendants()->isA()->name: ") + ((AcRxObject*) item->isA()->myParent()->descendants())->isA()->name(), tabLevel); //returns 0
                    //myAcutPrintLine(std::wstring(L"item->isA(): ") + std::to_wstring((uintptr_t)item->isA()), tabLevel);
                    myAcutPrintLine(std::wstring(L"ancestors of item->isA()->myParent()->descendants()->isA(): "), tabLevel);

                    tabLevel++;
                    std::list<AcRxClass*> ancestryList;
                    ancestryList = std::list<AcRxClass*>();
                    ancestor = ((AcRxObject*)item->isA()->myParent()->descendants())->isA();
                    while (true) {
                        ancestryList.push_front(ancestor);
                        if (ancestor == AcRxObject::desc()) { break; }
                        ancestor = ancestor->myParent();
                    }

                    int i = 0;
                    for (decltype(ancestryList)::iterator it = ancestryList.begin(); it != ancestryList.end(); ++it) {
                        myAcutPrintLine(std::wstring(L"ancestor ") + std::to_wstring(i) + L" class: " + (*it)->name(), tabLevel);
                        i++;
                    }

                    /* ancestors of item->isA()->myParent()->descendants()->isA() :
                            ancestor 0 class : AcRxObject
                            ancestor 1 class : AcRxSet
                            ancestor 2 class : AcRxImpSet
                    */ // neither AcRxSet nor AcRxImpSet is mentioned in the documentation.

                    AcArray<AcRxClass>* arrayOfRxClassesP;
                    //AcRxObject* mysteryDescendantsObject = ((AcRxObject*) item->isA()->myParent()->descendants() );
                    AcRxObject* mysteryDescendantsObject = ((AcRxObject*)item->isA()->myParent()->myParent()->descendants());

                    arrayOfRxClassesP = (AcArray<AcRxClass>*) mysteryDescendantsObject;

                    myAcutPrintLine(std::wstring(L"arrayOfRxClassesP->length(): ") + std::to_wstring(arrayOfRxClassesP->length()), tabLevel);

                    AcRxClass* mysteryDescendantsClass = mysteryDescendantsObject->isA();
                    AcRxClass* mysteryAcRxSetClass = mysteryDescendantsObject->isA()->myParent();
                    //myAcutPrintLine(std::wstring(L"mysteryDescendantsObject->isA()->name(): ") + mysteryDescendantsObject->isA()->name(), tabLevel);
                    myAcutPrintLine(std::wstring(L"mysteryDescendantsClass->name(): ") + mysteryDescendantsClass->name(), tabLevel);
                    myAcutPrintLine(std::wstring(L"mysteryAcRxSetClass->name(): ") + mysteryAcRxSetClass->name(), tabLevel);
                    myAcutPrintLine(std::wstring(L"mysteryAcRxSetClass->members(): ") + std::to_wstring((uintptr_t)mysteryAcRxSetClass->members()), tabLevel); //prints 0

                    // how can we (if at all) iterate over all instances of AcRxClass to completely traverse the entire taxonomy of AcRxObject descendants?

                    //AcRxMemberCollection* memberCollectionP = ((AcRxObject*)item->isA()->myParent()->descendants())->isA()->members();

                    //for (int i = 0; i < memberCollectionP->count(); i++) {
                    //    myAcutPrintLine(std::wstring(L"member ") + memberCollectionP->getAt(i)->isA()->name(), tabLevel);
                    //}

                    tabLevel--;
                }
                    
                myAcutPrintLine(std::wstring(L"item->isA()->members(): ") + std::to_wstring((uintptr_t)item->isA()->members()), tabLevel); //prints 0

                myAcutPrintLine(objectIdToString(item->objectId()), tabLevel);
                tabLevel++;
                AcDbObjectId ownerId = item->ownerId();
                while (true) {
                    myAcutPrintLine(L"is owned by " + objectIdToString(ownerId), tabLevel);
                    if (ownerId == AcDbObjectId::kNull) { break; }
                    AcDbObject* owner;
                    if( acdbOpenObject(owner, ownerId, AcDb::kForRead) != Acad::eOk) {
                        myAcutPrintLine(L"unable to open owner.", tabLevel);
                        break;
                    }
                    else {
                        ownerId = owner->ownerId();
                    }
                }
                tabLevel--;


                tabLevel--;
            }
            
            tabLevel--;
        }
        tabLevel--;
        pExtensionDictionary->close();
    }

    
    


    AcDbBlockTableRecordIterator* pBlockTableRecordIterator;
    pBlockTableRecord->newIterator(pBlockTableRecordIterator);
    AcDbEntity* pEntity;
    for (pBlockTableRecordIterator->start(); !pBlockTableRecordIterator->done(); pBlockTableRecordIterator->step()) {
        pBlockTableRecordIterator->getEntity(pEntity, AcDb::kForRead);
        AcDbHandle handle;
        
        //pEntity->getAcDbHandle(handle);
        handle = pEntity->objectId().handle();
        // of the above two statements, which seem to produce an equivalent effect, the latter seems cleaner to me.

        acutPrintf(
            _T("classname: %s, handle: %s\n"), 
            pEntity->isA()->name(),
            handleToString(handle)
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
