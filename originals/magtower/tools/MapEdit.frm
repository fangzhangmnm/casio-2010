VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   9000
   ClientLeft      =   120
   ClientTop       =   450
   ClientWidth     =   11400
   LinkTopic       =   "Form1"
   ScaleHeight     =   9000
   ScaleWidth      =   11400
   StartUpPosition =   3  '´°¿ÚÈ±Ê¡
   Begin VB.CommandButton CmdClear 
      Caption         =   "Clear"
      Height          =   495
      Left            =   8160
      TabIndex        =   5
      Top             =   2160
      Width           =   1575
   End
   Begin VB.CommandButton OutputPicture 
      Caption         =   "OutputPicture"
      Height          =   615
      Left            =   8160
      TabIndex        =   4
      Top             =   1440
      Width           =   1575
   End
   Begin VB.Timer Timer1 
      Interval        =   200
      Left            =   10080
      Top             =   240
   End
   Begin VB.CommandButton CmdOutputMap 
      Caption         =   "OutputMap"
      Height          =   735
      Left            =   8160
      TabIndex        =   2
      Top             =   600
      Width           =   1575
   End
   Begin VB.PictureBox Picture2 
      AutoRedraw      =   -1  'True
      Height          =   1455
      Left            =   0
      Picture         =   "MapEdit.frx":0000
      ScaleHeight     =   93
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   533
      TabIndex        =   1
      Top             =   6840
      Width           =   8055
   End
   Begin VB.PictureBox Picture1 
      AutoRedraw      =   -1  'True
      Height          =   6615
      Left            =   0
      ScaleHeight     =   437
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   533
      TabIndex        =   0
      Top             =   0
      Width           =   8055
   End
   Begin VB.Label Label1 
      Caption         =   "Label1"
      Height          =   375
      Left            =   8160
      TabIndex        =   3
      Top             =   0
      Width           =   1335
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim map(10, 10) As Byte
Dim clock As Integer
Dim brush As Integer

Private Sub CmdClear_Click()
    Dim i As Integer, j As Integer, fill As Byte
    fill = InputBox("Which Title?")
    For j = 0 To 10
    For i = 0 To 10
    map(i, j) = fill
    Next i
    Next j
End Sub

Private Sub CmdOutputMap_Click()
    Dim i As Integer, j As Integer, s As String
    
    s = "{" & vbCrLf
    For j = 0 To 10
        s = s & "{"
        For i = 0 To 10
            s = s & vbTab & map(i, j)
            If i < 10 Then s = s & ","
        Next i
        s = s & "}"
        If j < 10 Then s = s & "," & vbCrLf
    Next j
    s = s & vbCrLf & "}"
    Clipboard.Clear
    Clipboard.SetText s
End Sub

Private Sub Form_Load()
    Dim i As Integer, j As Integer, pid As Integer, b As Byte
    For j = 0 To 10
    For i = 0 To 10
    map(i, j) = 0
    Next i
    Next j
    brush = 0
    Label1.Caption = brush
    Draw
End Sub
Sub Draw()
    Dim i As Integer, j As Integer, pid As Integer
    Picture1.Cls

    For j = 0 To 10
        For i = 0 To 10
            pid = MapToPid(map(i, j), clock)
            Picture1.PaintPicture Picture2.Picture, 32 * i, 32 * j, 32, 32, (pid Mod 32) * 17 + 1, Int(pid / 32) * 17 + 1, 16, 16
        Next i
    Next j
    For i = 0 To 10
        Picture1.Line (0, 32 * i)-(32 * 11, 32 * i), vbBlue
        Picture1.Line (0, 32 * i + 31)-(32 * 11, 32 * i + 31), vbBlue
        Picture1.Line (32 * i, 0)-(32 * i, 32 * 11), vbBlue
        Picture1.Line (32 * i + 31, 0)-(32 * i + 31, 32 * 11), vbBlue
    Next i
End Sub
Function MapToPid(Index As Byte, clock As Integer) As Integer
    If Index >= 200 Then
        MapToPid = Index - 200
        If clock Then MapToPid = MapToPid + 32
    ElseIf Index >= 100 Then
        MapToPid = Index - 100 + 64
    ElseIf Index >= 50 Then
        MapToPid = (Index - 50) * 2 + 96
        If clock Then MapToPid = MapToPid + 1
    Else
        MapToPid = Index + 128
        If Index = 10 Or Index = 13 Or Index = 15 Then
            If clock Then MapToPid = MapToPid + 1
        End If
    End If
End Function

Private Sub OutputPicture_Click()
    Dim pid As Integer
    Dim mat(5120) As Byte, b As Byte
    Dim s As String
    Dim i As Integer, x As Integer, y As Integer
    s = ""
    For pid = 0 To 159
        For y = Int(pid / 32) * 17 + 1 To Int(pid / 32) * 17 + 16 Step 1
            For x = 0 To 1
                b = 0
                For i = 0 To 7 Step 1
                    b = b * 2
                    If Picture2.Point((pid Mod 32) * 17 + 1 + 8 * x + i, y) = vbBlack Then b = b + 1
                Next i
                s = s & "0x" & Format(Hex(b), "00")
                If x < 9 Then s = s & ","
            Next x
            s = s & vbCrLf
        Next y
        s = s & vbCrLf
    Next pid
    Clipboard.Clear
    Clipboard.SetText s
End Sub

Private Sub Picture1_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
    On Error Resume Next
    map(Int(x / 32), Int(y / 32)) = brush
    Draw
End Sub

Private Sub Picture2_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
    Dim i As Integer, j As Integer
    i = Int(x / 17): j = Int(y / 17)
    Select Case j
    Case 0, 1:
        brush = 200 + i
    Case 2:
        brush = 100 + i
    Case 3:
        brush = 50 + i / 2
    Case 4:
        brush = i
    End Select
    Label1.Caption = brush
End Sub

Private Sub Timer1_Timer()
    clock = Not clock
    Draw
End Sub
