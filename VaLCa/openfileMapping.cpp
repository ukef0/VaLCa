#include <windows.h>
#include <stdio.h>
int sharedMemSize = 1024 * 64;

HANDLE hFM = OpenFileMapping(FILE_MAP_READ, FALSE, L"Sakura");
//LPWSTR lpStr = (LPWSTR)MapViewOfFile(hFM, FILE_MAP_READ, 0, 0, 0);
char* lpStr = (char*)MapViewOfFile(hFM, FILE_MAP_READ, 0, 0, sharedMemSize);



'// ���ɍ쐬����Ă���FileMappingObject(FMO)��ǂݎ��Ӱ�ނŊJ��
Dim hFm As Long
hFm = OpenFileMapping(FILE_MAP_READ, 0, "Sakura")
'// �擾��������ق���A��؂�ϯ�߂���
Dim lpFile As Long
lpFile = MapViewOfFile(hFm, FILE_MAP_READ, 0, 0, 0)
'// FMO"Sakura"�̐擪4�޲Ăɂ́AFMO�̻��ނ��i�[����Ă���(�g�p�����)
Dim lSizeFmo As Long
CopyMemory lSizeFmo, ByVal lpFile, 4
'// �޲Ĕz����ް����i�[����B
'// ���ނ́A�uFMO�S����-4�v(�d�l�����)
Dim baWork() As Byte
ReDim baWork(lSizeFmo)
CopyMemory baWork(0), ByVal lpFile + 4, lSizeFmo - 4
'// �޲Ĕz��𕶎���ɕϊ�����
'// �e�ް���CR+LF�ŋ�؂��Ă���̂�(�d�l�����)�ASplit�ŕ������Ă��
Dim saData() As String
saData = Split(StrConv(baWork, vbUnicode), vbCrLf)
'//
'// �擾�����S�Ă̴��؂ɑ΂��A�������s��
'//
Dim i As Integer
Dim st1 As Integer, st2 As Integer
Dim sEntryName As String
Dim sValue     As String
For i = 0 To UBound(saData) - 1

'// ���ؖ��̎擾
st1 = InStr(1, saData(i), ".")
st2 = InStr(st1 + 1, saData(i), Chr(1))
sEntryName = Mid(saData(i), st1 + 1, st2 - st1 - 1)

'// �l�̎擾
sValue = Mid(saData(i), st2 + 1)

'// �\���̌���
MsgBox "���ؖ�[" & sEntryName & "]=" & sValue, , "���"
Next i
'// ϯ�߂������������ƁA����ق����
Call UnmapViewOfFile(lpFile)
Call CloseHandle(hFm)