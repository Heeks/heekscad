import wx
from wx.glcanvas import GLCanvas as wxGLCanvas
import sys
import math
import HeeksCAD
from Mouse import MouseEventFromWx

class myGLCanvas(wxGLCanvas):
   def __init__(self, parent):
      wxGLCanvas.__init__(self, parent,-1)
      self.Bind(wx.EVT_PAINT, self.OnPaint)
      self.Bind(wx.EVT_SIZE, self.OnSize)
      self.Bind(wx.EVT_MOUSE_EVENTS, self.OnMouse)
      self.Bind(wx.EVT_ERASE_BACKGROUND, self.OnEraseBackground)
      self.Bind(wx.EVT_MENU, self.OnMenu, None, 10000, 12000)
      self.viewport = HeeksCAD.Viewport()
      self.Resize()
        
   def OnSize(self, event):
      self.Resize()
      event.Skip()

   def OnMenu(self, event):
      index = event.GetId() - 10000
      tool = self.tools[index]
      tool.Run()

   def AppendToolsToMenu(self, menu, tools):
      for tool in tools:
         if tool.IsSeparator():
            menu.AppendSeparator()
         elif tool.IsAToolList():
            sub_menu = wx.Menu()
            self.AppendToolsToMenu(sub_menu, tool.GetChildTools())
            menu.AppendMenu(wx.ID_ANY, tool.GetTitle(), sub_menu)
         else:
            item = wx.MenuItem(menu, 10000 + self.next_tool_id, text = tool.GetTitle(), help = tool.GetToolTip())
            str = tool.BitmapPath()
            if len(str)>0:
               image = wx.Image('bitmaps/' + str + '.png')
               image.Rescale(24, 24)
               item.SetBitmap(wx.BitmapFromImage(image))
            menu.AppendItem(item)
            self.next_tool_id = self.next_tool_id + 1
            self.tools.append(tool)
        
   def OnMouse(self, event):
      if event.RightUp():
         tools = HeeksCAD.GetDropDownTools(event.GetX(), event.GetY(), False, True, event.m_controlDown)
         if len(tools) > 0:
            self.next_tool_id = 0
            self.tools = []
            menu = wx.Menu()
            self.AppendToolsToMenu(menu, tools)
            self.PopupMenu(menu)
      else:
         e = MouseEventFromWx(event)
         self.viewport.OnMouseEvent(e)

      if self.viewport.m_need_update: self.Update()
      if self.viewport.m_need_refresh: self.Refresh()
      event.Skip()
        
   def OnEraseBackground(self, event):
      pass # Do nothing, to avoid flashing on MSW
        
   def Resize(self):
      s = self.GetClientSize()
      self.viewport.WidthAndHeightChanged(s.GetWidth(), s.GetHeight())
      self.Refresh()

   def OnPaint(self, event):
      dc = wx.PaintDC(self)
      self.SetCurrent()
      self.viewport.glCommands()
      self.SwapBuffers()
      self.viewport.DrawFront()
      return
