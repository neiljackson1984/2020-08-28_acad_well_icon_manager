using System;
using Autodesk.AutoCAD.DatabaseServices;
using Autodesk.AutoCAD.Runtime;
using Autodesk.AutoCAD.Geometry;
using Autodesk.AutoCAD.ApplicationServices;
using System.Reflection;

using System.IO;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;
using Autodesk.AutoCAD.EditorInput;
using Autodesk.AutoCAD.GraphicsInterface;

[assembly: Autodesk.AutoCAD.Runtime.ExtensionApplication(typeof(acad_well_icon_manager.Acad_well_icon_managerApp))]
[assembly: Autodesk.AutoCAD.Runtime.CommandClass(typeof(acad_well_icon_manager.Acad_well_icon_managerCommands))]

namespace acad_well_icon_manager
{

    public class Acad_well_icon_managerApp : Autodesk.AutoCAD.Runtime.IExtensionApplication
    {
        public Acad_well_icon_managerApp()
        {


        }

        public void Initialize()
        {
            Acad_well_icon_managerCommands.DoIt();

        }

        public void Terminate()
        {


        }

    }

    public class Acad_well_icon_managerCommands
    {
        [CommandMethod("acad_well_icon_manager_make")]
        static public void DoIt()
        {
            Document document = Application.DocumentManager.MdiActiveDocument;
            Database database = document.Database;
            Autodesk.AutoCAD.DatabaseServices.TransactionManager transactionManager = database.TransactionManager;
            using (Transaction transaction = transactionManager.StartTransaction())
            {
                document.Editor.WriteMessage("HOORAY6\n");
                //BlockTable blockTable = (BlockTable)transactionManager.GetObject(database.BlockTableId, OpenMode.ForRead, openErased: false);
                //BlockTableRecord blockTableRecord = (BlockTableRecord)transactionManager.GetObject(blockTable[BlockTableRecord.ModelSpace], OpenMode.ForWrite, false);
                //foreach (Autodesk.AutoCAD.DatabaseServices.ObjectId objectId in blockTableRecord)
                //{
                //    DBObject dbObject = objectId.GetObject(OpenMode.ForRead);
                //    document.Editor.WriteMessage("type: " + dbObject.GetType() + "\n");
                //    //if (dbObject.GetType() == typeof(MLeader))
                //    //{
                //    //    document.Editor.WriteMessage("We found a multileader.\n");
                //    //    MLeader mLeader = (MLeader)dbObject;

                //    //    // damn: the .net api does not expose the functions related to mleader style overrides.
                //    //    //mLeader.isoverride(MLeader.PropertyOverrideType.kLeaderLineType);
                //    //    //MLeader.setOverride(MLeader.PropertyOverrideType.kLeaderLineType, false);
                //    //    //kLeaderLineType

                //    //    //ErrorStatus result = setOverride(mLeader.UnmanagedObject, 16, false);

                //    //    //bool result = isOverride(mLeader.UnmanagedObject, 16);
                //    //    //document.Editor.WriteMessage("result: " + result + "\n");
                //    //}


                //}
            }

        }
		

	}


}