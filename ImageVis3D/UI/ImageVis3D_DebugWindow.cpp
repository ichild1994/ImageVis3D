/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


//!    File   : ImageVis3D_DebugWindow.cpp
//!    Author : Jens Krueger
//!             SCI Institute
//!             University of Utah
//!    Date   : September 2008
//
//!    Copyright (C) 2008 SCI Institute

#include <sstream>
#include "../Tuvok/Basics/SystemInfo.h"
#include "../Tuvok/Basics/SysTools.h"
#include "../Tuvok/Renderer/GPUMemMan/GPUMemMan.h"
#include "../Tuvok/IO/IOManager.h"
#include "../Tuvok/Scripting/Scripting.h"
#include "ImageVis3D.h"
#include <QtGui/QImageReader>

using namespace std;

void MainWindow::ShowVersions() {
  std::ostringstream versions;
  versions << "Tuvok Version: " << TUVOK_VERSION << " " << TUVOK_VERSION_TYPE
           << " " << TUVOK_DETAILS << "\n"
#ifdef TUVOK_SVN_VERSION
           << "SVN Version: " << int(TUVOK_SVN_VERSION) << "\n "
#endif
           << "ImageVis3D Version: " << IV3D_VERSION " " << IV3D_VERSION_TYPE << " "
#ifdef IV3D_SVN_VERSION
           << "\nSVN Version: " << int(IV3D_SVN_VERSION) << "\n "
#endif
           << "Qt Version: " << QT_VERSION_STR;
  m_MasterController.DebugOut()->printf(versions.str().c_str());
}

void MainWindow::ShowGPUInfo(bool bWithExtensions) {
#if defined(_WIN32) && defined(USE_DIRECTX)
  if (DynamicDX::IsInitialized()) {
    std::ostringstream d3dver;
    d3dver << "Direct3DX10 Version " << DynamicDX::GetD3DX10Version();
    m_MasterController.DebugOut()->printf(d3dver.str().c_str());
  } else
    m_MasterController.DebugOut()->printf("DirectX 10 not initialzed");
#endif

  if (RenderWindow::GetVendorString() == "") {
    m_MasterController.DebugOut()->printf("For the GL specs please open a GL renderwindow first!");
  } else {
    AbstrDebugOut *dbg = m_MasterController.DebugOut();

    dbg->printf(RenderWindow::GetVendorString().c_str());
    {
      std::ostringstream tex_size;
      tex_size << "Maximum Volume size "
               << int(RenderWindow::GetMax3DTexDims())
               << ((RenderWindow::Get3DTexInDriver()) ?
                   " via 3D textures" :
                   " via 2D texture stacks");
      dbg->printf(tex_size.str().c_str());
    }

    if (!bWithExtensions) return;

    if (RenderWindowGL::GetExtString() != "") {
      m_MasterController.DebugOut()->printf("Supported GL extensions:");

      vector< string > vExtensions = SysTools::Tokenize(RenderWindowGL::GetExtString());
      std::ostringstream ext;
      for (size_t i = 0;i<vExtensions.size();i++) {
        ext << "  " << vExtensions[i] << "\n";
      }
      ext << vExtensions.size() << " extensions found.";
      dbg->printf(ext.str().c_str());
    }
  }
}


void MainWindow::ListSupportedImages() {
  m_MasterController.DebugOut()->printf("Supported image formats are:");
  QList<QByteArray> listImageFormats = QImageReader::supportedImageFormats();
  for (int i = 0;i<listImageFormats.size();i++) {
    QByteArray imageFormat = listImageFormats[i];
    QString qStrImageFormat(imageFormat);
    string strImageFormat = "  " + qStrImageFormat.toStdString();
    m_MasterController.DebugOut()->printf(strImageFormat.c_str());
  }
}

void MainWindow::ListSupportedVolumes() {
  m_MasterController.DebugOut()->printf("Supported Volume formats are:");

  std::vector<tConverterFormat> conv = m_MasterController.IOMan()->GetFormatList();

  for (size_t i=0; i < conv.size(); i++) {
    string strVolumeFormats = "  " + tr1::get<0>(conv[i]) + " " + tr1::get<1>(conv[i]);
    if (!tr1::get<2>(conv[i])) 
      strVolumeFormats += " (Readonly)";
    m_MasterController.DebugOut()->printf(strVolumeFormats.c_str());
  }

}


void MainWindow::ListSupportedGeometry() {
  m_MasterController.DebugOut()->printf("Supported Geometry formats are:");

  std::vector<tConverterFormat> conv = m_MasterController.IOMan()->GetGeoFormatList();

  for (size_t i=0; i < conv.size(); i++) {
    string strGeoFormats = "  " + tr1::get<0>(conv[i]) + " " + tr1::get<1>(conv[i]);
      if (!tr1::get<2>(conv[i])) 
        strGeoFormats += " (Readonly)";
    m_MasterController.DebugOut()->printf(strGeoFormats.c_str());
  }
}

void MainWindow::ShowSysInfo() {
  std::ostringstream sysinfo;
  const GPUMemMan *mm = m_MasterController.MemMan();

  const size_t one_megabyte = 1024*1024;
  UINT64 cpu_mem = mm->GetCPUMem();
  UINT64 max_cpu_mem = m_MasterController.SysInfo()->GetMaxUsableCPUMem();

  sysinfo << "This is a " << m_MasterController.MemMan()->GetBitWidthMem() << "bit build\n"
          << "CPU Memory: Total " << cpu_mem / one_megabyte << " MB, "
          << "Usable " << max_cpu_mem << "MB\n"
          << "    Used: " << mm->GetAllocatedCPUMem()/one_megabyte << " MB "
          << "(" << mm->GetAllocatedCPUMem() << " bytes)\n";

  if (m_MasterController.MemMan()->GetAllocatedCPUMem() <
      m_MasterController.MemMan()->GetCPUMem()) {
    sysinfo << "    Available: "
            << (mm->GetCPUMem() - mm->GetAllocatedCPUMem())/one_megabyte
            << "\n";
  }

  sysinfo << "GPU Memory: Total " << mm->GetGPUMem()/one_megabyte << " MB, "
          << "Usable "
          << m_MasterController.SysInfo()->GetMaxUsableGPUMem()/one_megabyte
          << "    Used: " << mm->GetAllocatedGPUMem()/one_megabyte << " MB ("
          << mm->GetAllocatedGPUMem() << " Bytes)";

  if (m_MasterController.MemMan()->GetAllocatedGPUMem() <
      m_MasterController.MemMan()->GetGPUMem()) {
    sysinfo << "    Available: "
            << (mm->GetGPUMem() - mm->GetAllocatedGPUMem())/one_megabyte
            << "\n";
  }
  m_MasterController.DebugOut()->printf(sysinfo.str().c_str());
}

void MainWindow::ClearDebugWin() {
  listWidget_DebugOut->clear();
}

void MainWindow::SetDebugViewMask() {
  m_MasterController.DebugOut()->SetOutput(checkBox_ShowDebugErrors->isChecked(),
                                           checkBox_ShowDebugWarnings->isChecked(),
                                           checkBox_ShowDebugMessages->isChecked(),
                                           checkBox_ShowDebugOther->isChecked());
}

void MainWindow::GetDebugViewMask() {

  bool bShowErrors;
  bool bShowWarnings;
  bool bShowMessages;
  bool bShowOther;

  m_MasterController.DebugOut()->GetOutput(bShowErrors,
                      bShowWarnings,
                      bShowMessages,
                      bShowOther);

  checkBox_ShowDebugErrors->setChecked(bShowErrors);
  checkBox_ShowDebugWarnings->setChecked(bShowWarnings);
  checkBox_ShowDebugMessages->setChecked(bShowMessages);
  checkBox_ShowDebugOther->setChecked(bShowOther);
}

void MainWindow::ParseAndExecuteDebugCommand() {
  bool bTemp = GetDebugOut()->ShowMessages();
  GetDebugOut()->SetShowMessages(true);

  m_MasterController.ScriptEngine()->ParseLine(lineEdit_DebugCommand->text().toStdString());

  GetDebugOut()->SetShowMessages(bTemp);
  if (listWidget_DebugOut->count() > 0)
    listWidget_DebugOut->setCurrentRow(listWidget_DebugOut->count()-1);

  lineEdit_DebugCommand->clear();
}
