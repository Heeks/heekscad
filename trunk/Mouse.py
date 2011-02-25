import wx
import HeeksCAD

def MouseEventFromWx(w):
   e = HeeksCAD.MouseEvent()
   if w.LeftDown():
      e.m_event_type = 1
   elif w.LeftUp():
      e.m_event_type = 2
   elif w.LeftDClick():
      e.m_event_type = 3
   elif w.RightDown():
      e.m_event_type = 4
   elif w.RightUp():
      e.m_event_type = 5
   elif w.MiddleDown():
      e.m_event_type = 6
   elif w.MiddleUp():
      e.m_event_type = 7
   elif w.Dragging() or w.Moving():
      e.m_event_type = 8
   elif w.GetWheelRotation():
      e.m_event_type = 9
   
   e.m_x = w.m_x
   e.m_y = w.m_y 
   e.m_leftDown = w.m_leftDown 
   e.m_middleDown = w.m_middleDown 
   e.m_rightDown = w.m_rightDown 
   e.m_controlDown = w.m_controlDown 
   e.m_shiftDown = w.m_shiftDown 
   e.m_altDown = w.m_altDown 
   e.m_metaDown = w.m_metaDown 
   e.m_wheelRotation = w.m_wheelRotation 
   e.m_wheelDelta = w.m_wheelDelta 
   e.m_linesPerAction = w.m_linesPerAction 

   return e
