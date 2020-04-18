#include <windows.h>
#include <stdio.h>
int sharedMemSize = 1024 * 64;

HANDLE hFM = OpenFileMapping(FILE_MAP_READ, FALSE, L"Sakura");
//LPWSTR lpStr = (LPWSTR)MapViewOfFile(hFM, FILE_MAP_READ, 0, 0, 0);
char* lpStr = (char*)MapViewOfFile(hFM, FILE_MAP_READ, 0, 0, sharedMemSize);



'// 既に作成されているFileMappingObject(FMO)を読み取りﾓｰﾄﾞで開く
Dim hFm As Long
hFm = OpenFileMapping(FILE_MAP_READ, 0, "Sakura")
'// 取得したﾊﾝﾄﾞﾙから、ﾒﾓﾘへﾏｯﾌﾟする
Dim lpFile As Long
lpFile = MapViewOfFile(hFm, FILE_MAP_READ, 0, 0, 0)
'// FMO"Sakura"の先頭4ﾊﾞｲﾄには、FMOのｻｲｽﾞが格納されている(使用書より)
Dim lSizeFmo As Long
CopyMemory lSizeFmo, ByVal lpFile, 4
'// ﾊﾞｲﾄ配列にﾃﾞｰﾀを格納する。
'// ｻｲｽﾞは、「FMO全ｻｲｽﾞ-4」(仕様書より)
Dim baWork() As Byte
ReDim baWork(lSizeFmo)
CopyMemory baWork(0), ByVal lpFile + 4, lSizeFmo - 4
'// ﾊﾞｲﾄ配列を文字列に変換する
'// 各ﾃﾞｰﾀはCR+LFで区切られているので(仕様書より)、Splitで分解してやる
Dim saData() As String
saData = Split(StrConv(baWork, vbUnicode), vbCrLf)
'//
'// 取得した全てのｴﾝﾄﾘに対し、調査を行う
'//
Dim i As Integer
Dim st1 As Integer, st2 As Integer
Dim sEntryName As String
Dim sValue     As String
For i = 0 To UBound(saData) - 1

'// ｴﾝﾄﾘ名の取得
st1 = InStr(1, saData(i), ".")
st2 = InStr(st1 + 1, saData(i), Chr(1))
sEntryName = Mid(saData(i), st1 + 1, st2 - st1 - 1)

'// 値の取得
sValue = Mid(saData(i), st2 + 1)

'// 表示の結果
MsgBox "ｴﾝﾄﾘ名[" & sEntryName & "]=" & sValue, , "情報"
Next i
'// ﾏｯﾌﾟを解除したあと、ﾊﾝﾄﾞﾙを閉じる
Call UnmapViewOfFile(lpFile)
Call CloseHandle(hFm)