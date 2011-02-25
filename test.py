import HeeksCAD

import wx
import wx.aui
import sys
from GraphicsCanvas import myGLCanvas

HeeksCAD.init()
HeeksCAD.OnCubeButton()
HeeksCAD.redraw()

save_out = sys.stdout
save_err = sys.stderr

app = wx.App()

sys.stdout = save_out
sys.stderr = save_err

# make a wxWidgets application
frame= wx.Frame(None, -1, 'CAM ( Computer Aided Manufacturing ) from DXF files')
aui_manager = wx.aui.AuiManager()
aui_manager.SetManagedWindow(frame)

g = myGLCanvas(frame)

aui_manager.AddPane(g, wx.aui.AuiPaneInfo().Name('graphics').Center())

frame.Center()
aui_manager.Update()
frame.Show()

app.MainLoop()
