//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"

#include <unordered_map>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

std::string myPluginPath = "plugins/NppCustomSnippet/";
std::string configFile = "ConfigMeta.txt";
/*
config_meta�ﶨ����� �ļ���չ�� �� �ļ��Զ������� ��ӳ���
ÿ���Զ����������ͣ���Ӧһ�� ����.txt ��json�ļ���
����洢�ľ��� snippet cmd


example:

config_meta = 
{
	"txt" => "normal",
	"py" => "python",
	"js" => "javascript",
	"ts" => typescript,
	"vue" => "vue"
}
*/

json config_meta = {};
json sence = {};

//��һ�γɹ����ɵĴ��������
std::string LastSnippetName = "";
//��һ�����ɵĴ����ĳ���
size_t LastSnippetLength = 0;

//��ǰ�ĵ���ʲôsence ��
std::string current_sence_name = "";


bool SaveData(std::string sence_name)
{
	std::ofstream f_file(myPluginPath + sence_name + ".txt");
    f_file << sence.dump(1, '\t');
	return true;
}

bool LoadSenceData(std::string sence_name)
{
	std::ifstream f_file(myPluginPath + sence_name + ".txt");
	if(! f_file.is_open())
		return false;
    sence = json::parse(f_file);
	return true;
}

//================

// �����Ҫת��Ϊ UTF-8 �� std::string��������ĳЩ��ƽ̨�⣩
std::string WStringToUTF8(const std::wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Str(size + 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], size, nullptr, nullptr);
    return utf8Str;
}

std::string wstring_to_utf8(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();

	// ��һ�ε��û�ȡ��Ҫ���ֽ��������� \0��
	int size_needed = WideCharToMultiByte(
		CP_UTF8,                // Ŀ����룺UTF-8
		0,                      // ת����־
		wstr.c_str(),           // �����ַ���
		(int)wstr.size(),       // ���볤��
		nullptr,                // ������������Ȳ�����
		0,                      // �����������С
		nullptr, nullptr        // Ĭ������ַ�
	);

	if (size_needed <= 0) {
		throw std::runtime_error("WideCharToMultiByte failed");
	}

	std::string result(size_needed, 0);

	// �ڶ��ε��ã�����ת��
	WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr.c_str(),
		(int)wstr.size(),
		&result[0],
		size_needed,
		nullptr, nullptr
	);

	return result;
}

//�õ���չ��
std::string GetExtName(std::string &full_path)
{
	std::string temp = "";
	size_t len = full_path.size();
	int index = int(len);

	while (index >= 0)
	{
		index--;
		if (full_path[index] == '.')
			break;
		temp.push_back(full_path[index]);
	}


	std::reverse(temp.begin(), temp.end() );

	return temp;
}

std::string api_GetExtName()
{
	wchar_t fullPath[MAX_PATH] = {0};
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)fullPath);
	std::string temp = wstring_to_utf8(fullPath);
	std::string ext = GetExtName(temp);
	return ext;
}


//============

//���ص�ǰ��sence
bool api_LoadSence()
{
	std::string ext = api_GetExtName();
	//std::ofstream f_file(myPluginPath + "ext_log.txt");
	//f_file << ext;
	
	
	if(! config_meta.contains(ext))
	{
		std::string msg = "" ;
		msg = "You didnt define the " + ext + " file type in ConfigMeta.txt map";
		::MessageBoxA(NULL, msg.c_str(), "woring", MB_OK);
		return false;
	}
		
	std::string sence_name = config_meta[ext];
	if(sence_name == current_sence_name)
	{
		//�����ǰ û�䣬֤�����Ѿ�����
		return true;
	}
	bool done = LoadSenceData(sence_name);
	if(! done)
	{
		std::string msg = "could not load sence name:" + sence_name;
		::MessageBoxA(NULL, msg.c_str(), "woring", MB_OK);
		return false;
	}
		
	current_sence_name = sence_name;
	return true;
}

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
    std::ifstream f_meta_config(myPluginPath + configFile);

    //�ļ������ڣ������Ǵ���һ��ģ��
    if (!f_meta_config.good())
    {
        config_meta["txt"] = "normal";
		config_meta["js"] = "javascript";
		config_meta["py"] = "python";
		config_meta["vue"] = "vue";
		
        sence["normal"]["test"]["params"] = json::array();
        sence["normal"]["test"]["snippetLines"] = 0;
        sence["normal"]["test"]["contentText"] = "";
		

        std::ofstream f_meta_config_template(myPluginPath + configFile);
        f_meta_config_template << config_meta.dump(1, '\t');

        std::ofstream f_normal(myPluginPath + "normal.txt");
        f_normal << sence.dump(1, '\t');
    }
    else
    {
       // f_meta_config >> config_meta;
		config_meta = json::parse(f_meta_config);
		
		LoadSenceData("normal");

       // std::ifstream f_normal(myPluginPath + "normal.txt");
        //f_normal >> sence["normal"];
		//LoadData("normal");
		//api_LoadSence();
    }
    
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, (TCHAR*)TEXT("Hello Notepad++"), hello, NULL, false);
    setCommand(1, (TCHAR*)TEXT("Hello (with dialog)"), helloDlg, NULL, false);
	setCommand(2, (TCHAR*)TEXT("AddCmd"), WhenAddSnippet, NULL, false);
	setCommand(3, (TCHAR*)TEXT("Snippet"), WhenSnippet, NULL, false);
	setCommand(4, (TCHAR*)TEXT("JumpCursor"), WhenJump, NULL, false);
	//Debug
	setCommand(5, (TCHAR*)TEXT("Debug"), Debug, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void hello()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
    ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)"Hello, Notepad++!");
}

void helloDlg()
{
    ::MessageBox(NULL, TEXT("Hello, Notepad++!"), TEXT("Notepad++ Plugin Template"), MB_OK);
}

//===���ǵĴ��뿪ʼ=====================================================
HWND getCurrentScintillaHandle()
{
    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    HWND hScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
    return hScintilla;
}


std::string GetCurrentLineString(HWND hScintilla) {
    // 1. ��ȡ�з�Χ��ʹ�� intptr_t ���� 64 λ��
    intptr_t currentPos = SendMessage(hScintilla, SCI_GETCURRENTPOS, 0, 0);
	//��ȡ�к�
    intptr_t linePos = SendMessage(hScintilla, SCI_LINEFROMPOSITION, currentPos, 0);
	
    intptr_t lineStartPos = SendMessage(hScintilla, SCI_POSITIONFROMLINE, linePos, 0);
    intptr_t lineEndPos = SendMessage(hScintilla, SCI_GETLINEENDPOSITION, linePos, 0);
	
	size_t byteSize = lineEndPos - lineStartPos + 1 ;
	//std::string lineStr(byteSize , '\0');
    std::vector<char> lineStr(byteSize, '\0');

    // 3. ���� SCI_GETTEXTRANGEFULL
    Sci_TextRangeFull text_range;	//����APIҪ��Ľṹ��
    text_range.chrg.cpMin = lineStartPos;
    text_range.chrg.cpMax = lineEndPos;
    text_range.lpstrText = lineStr.data();  
	
    SendMessage(hScintilla, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&text_range);
	
    std::string text(lineStr.data());
    return text;
}

std::string GetTextByRange(HWND hScintilla , Sci_Position from , Sci_Position to)
{
	size_t byteSize = to - from + 1 ;
    std::vector<char> buffer(byteSize, '\0');
	//����APIҪ��Ľṹ��
    Sci_TextRangeFull text_range;
    text_range.chrg.cpMin = from;
    text_range.chrg.cpMax = to;
    text_range.lpstrText = buffer.data();  
	
    ::SendMessage(hScintilla, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&text_range);
	
    std::string text(buffer.data());
    return text;
}

std::string GetSelectedText(HWND hScintilla)
{
	std::string text = "";
	Sci_Position startPos = SendMessage(hScintilla, SCI_GETSELECTIONSTART, 0, 0);
	Sci_Position endPos = SendMessage(hScintilla, SCI_GETSELECTIONEND, 0, 0);
	if(startPos > endPos)
	{
		return text;
	}
	
	size_t byteSize = endPos - startPos + 1;
	std::vector<char> lineStr(byteSize, '\0');
	
	Sci_TextRangeFull text_range;	//����APIҪ��Ľṹ��
    text_range.chrg.cpMin = startPos;
    text_range.chrg.cpMax = endPos;
    text_range.lpstrText = lineStr.data();  
	
    SendMessage(hScintilla, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&text_range);
	std::string text2(lineStr.data());
    return text2;
}



//���ҵ�һ����Ϊ�հ׷���λ�ã���Ϊ��ʼ��
size_t FindCodeLineStartPos(std::string &line)
{
	size_t index = 0;
	for(auto & ch : line)
	{
		if(std::isspace(ch) )
			index++;
		else
			return index;
	}
	
	return index;
}

//��һ���ַ�������ָ���ַ����ָ�Ϊ�ִ�����
std::vector<std::string> splitText( std::string str , char one_char)
{
	std::vector<std::string> parts;
	std::string temp = "";
	for(char &c : str)
	{
		if(c == one_char)
		{
			parts.push_back(temp);
			temp.clear();
			continue;
		}
		
		temp.push_back(c);
	}
	
	if(! temp.empty() )
	{
		parts.push_back(temp);
		temp.clear();
	}
	
	return parts;
}

//��һ���ַ�������ָ���ַ����ָ�Ϊ�ִ�����
//�޶��ָ�����������Ķ���Ϊһ��
std::vector<std::string> splitTextWithCountLimit( std::string str , char one_char , int stopSplitCount)
{
	std::vector<std::string> parts;
	std::string temp = "";
	int splited_count = 0;
	for(char &c : str)
	{
		if(c == one_char && splited_count < stopSplitCount)
		{
			parts.push_back(temp);
			temp.clear();
			splited_count++;
		}
		else
			temp.push_back(c);
	}
	
	if(! temp.empty() )
	{
		parts.push_back(temp);
		temp.clear();
	}
	
	return parts;
}

//��һ���ַ�����ȥ��ǰ�հ׷��������µ��ַ���
std::string TrimHeader(std::string &text)
{
	size_t start_pos = FindCodeLineStartPos(text);
	std::string text2 = text.substr(start_pos);
	return text2;
}

int GetTextLineCount(std::string &text)
{
	int count = 0;
	for(auto & ch : text)
	{
		if(ch == '\n')
			count++;
	}
	
	return count + 1;
}

void RemoveTextSlashR(std::string &text)
{
	text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
}

std::string ConvertToWindowsLineEndings(std::string& input)
{
    std::string output = "";
	for(auto &ch : input)
	{
		if(ch == '\n')
		{
			output.push_back('\r');
			output.push_back(ch);
		}
		else
			output.push_back(ch);
	}
	
    return output;
}

//Ϊÿһ����ӻ�׼����
std::string AddBaseIdentation(std::string& input , std::string &Identation)
{
    std::string output = "";
	for(auto &ch : input)
	{
		if(ch == '\n')
		{
			output.push_back(ch);
			output += Identation;
		}
		else
			output.push_back(ch);
	}
	
    return output;
}

size_t FindTwoCharPos(std::string & target , std::string &two_char)
{
	//ֻ��ѭ���������ڶ����ַ�������Խ��
	size_t len = target.size() - 1;
	for(size_t index = 0 ; index < len ; ++index)
	{
		bool found = target[index] == two_char[0] && target[index + 1] == two_char[1];
		if( found )
			return index;
	}

	return std::string::npos;
}

void CleanMark()
{
	LastSnippetName = "";
	LastSnippetLength = 0;
}

void WhenAddSnippet()
{
	api_LoadSence();
    /*
    ʵ��˼·��
    ��ȡ��ǰ�е��ı�
    ���ҵ�һ����Ϊ�հ׷���λ�ã���Ϊ��ʼ��
    һֱ������
    ����ʽ(����ո�)�ָ�Ϊ�ַ������飬
    �ҹؼ��ʣ�
    �滻
    */
    HWND curScintilla = getCurrentScintillaHandle();
	
	/*
    std::string lineText = GetCurrentLineString(curScintilla);
	size_t lineStartPos = FindCodeLineStartPos(lineText);
	std::string fixedLineText = lineText.substr(lineStartPos);
    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM) fixedLineText.c_str() );
	*/
	
	//��ȡѡ�е��ı�
	std::string selectedText = GetSelectedText(curScintilla);
	
	RemoveTextSlashR(selectedText);
	
	//ȥ��ǰ��Ŀհ׷�
	//�����룬���Ƕ���ģ���Ϊ����Ҫ���������ʱ�򶥸�д
	std::string fixed_selectedText = TrimHeader(selectedText);
	//��\n�ַ�����Ϊ3��
	auto line_parts = splitTextWithCountLimit(fixed_selectedText , '\n' , 2);
	
	//������Ҫ3�ж��壬1.ռλ�� 2.���� 3.�滻����
	if(line_parts.size() < 3)
	{
		::MessageBox(NULL, TEXT("We need at lest 3 line parts to define a command"), TEXT("wrong"), MB_OK);
		return;
	}
	
	auto cmdLine = line_parts[0];
	auto jumpLine = line_parts[1];
	auto snippetContent = line_parts[2];
	
	//ȥ��\r���ţ���������Ѿ�������ѡ���ı���ʱ������
	//RemoveTextSlashR(cursorMark);
	//RemoveTextSlashR(cmdLine);
	//RemoveTextSlashR(snippetContent);
	
	if(jumpLine.size() != 2)
	{
		::MessageBox(NULL, TEXT("jump line must be 2 character"), TEXT("wrong"), MB_OK);
		return;
	}
	
	//int snippetContent_line_count = GetTextLineCount(snippetContent);
	
	auto cmd_line_parts = splitText(cmdLine , ' ');
	std::string cmd_key = cmd_line_parts[0];
	
	size_t jump_pos = FindTwoCharPos(snippetContent , jumpLine);
	bool hasJump = jump_pos != std::string::npos;
	
	bool hasParam = cmd_line_parts.size() > 1;
	
	sence[cmd_key]["hasParam"] = hasParam;
	if(hasParam)
	{
		sence[cmd_key]["params"] = std::vector<std::string>(cmd_line_parts.begin() + 1, cmd_line_parts.end());
		//��Ҫ��� �ı��У��Ƿ��� ���� ��־��û�еĻ����� hasParam ì����
		std::string param_mark = "*{";
		size_t param_pos = FindTwoCharPos(snippetContent , param_mark);
		if(param_pos == std::string::npos)
		{
			::MessageBox(NULL, TEXT("you defined params , but your snippetContent has no params mark "), TEXT("wrong"), MB_OK);
			return ;
		}
	}
		
	
	sence[cmd_key]["cursorMark"] = jumpLine;
    //sence[cmd_key]["snippetLines"] = snippetContent_line_count;
	sence[cmd_key]["contentText"] = snippetContent;
	sence[cmd_key]["hasJump"] = hasJump;
	
	std::string sence_name = "normal";
	
	//����� ��Ҫ��ת�������������Ҫ�� snippetContent ĩβ���һ��������־
	//���磬�����ת������ =@  ������־���� =@@
	std::string end_mark = jumpLine + '@';
	
	SaveData(sence_name);
}

size_t FindJumpMarkPos(HWND hScintilla , Sci_Position fromPos , Sci_Position toPos , std::string &str)
{
	//�ٷ��涨�������ṹ��
	Sci_TextToFindFull tf = {};
	
	tf.chrg.cpMin = fromPos;
	tf.chrg.cpMax = toPos;
	tf.lpstrText = str.c_str();
	
	size_t jump_mark_pos = ::SendMessage(hScintilla, SCI_FINDTEXTFULL, 0, (LPARAM) &tf );
	return jump_mark_pos;
}

/*
std::string ParsedParamsContent(std::unordered_map<std::string,std::string> &dict , std::string &content)
{
	std::string fixed_content = "";
	std::string temp_key = "";
	size_t len = content.size();
	size_t stop_pos = len - 1;
	for(size_t index = 0 ; index < len ; ++index)
	{
		auto ch = content[index];
		auto ch2 = content[index + 1];
		
		//���index�ߵ� �����λ�ַ��ˣ��ǾͲ��ò���
		if(index < stop_pos && ch == '*' && ch2 == '{')
		{
			index += 2;//���� *{ �����ַ�
			while(content[index] != '}')
			{
				temp_key.push_back(content[index]);
			}
			fixed_content += dict[temp_key];
			temp_key.clear();
		}
		else
		{
			fixed_content.push_back(ch);
		}
	}
	
	return fixed_content;
}
*/

//��snippet content�����еĲ���ռλ����ȫ���滻�ɶ�Ӧ��ֵ
std::string ReplaceMarkToValue(std::string &content , std::unordered_map<std::string,std::string> &dict)
{
	size_t len = content.size();
	bool isInWordsRange = false;
	std::string out_text;
	out_text.reserve(len *2);
	std::string word_key = "";
	word_key.reserve(256);
	
	for(size_t index = 0 ; index < len; ++index )
	{
		auto ch = content[index];
		
		if(ch == '*' && index < len - 1 && (content[index + 1] == '{') )
		{
			index++;
			isInWordsRange = true;
			continue;
		}
		
		/*
		if(ch == '{' && index >0 && (content[index - 1] == '*') )
		{
			isInWordsRange = true;
			out_text.pop_back();
			continue;
		}
		*/
		
		//������� } ����
		if(isInWordsRange && ch == '}')
		{
			if(dict.contains(word_key))
			{
				std::string value = dict[word_key];
				out_text += value;
			}
			isInWordsRange = false;
			word_key.clear();
			continue;
		}
		
		if(isInWordsRange)
		{
			word_key.push_back(ch);
			continue;
		}
		
		//�ߵ�������ǳ�����ͨ�ַ��ˡ�
		out_text.push_back(ch);
	}
	
	return out_text;
}


void WhenSnippet()
{
	api_LoadSence();
	//bool done = api_LoadSence();
	/*
	if(done)
	{
		::MessageBox(NULL, TEXT("done"), TEXT("wrong"), MB_OK);
	}
	else
	{
		::MessageBox(NULL, TEXT("failed"), TEXT("wrong"), MB_OK);
	}
	*/
	HWND hScintilla = getCurrentScintillaHandle();
	
	//��ǰ���λ��(�ĵ���)
	Sci_Position cursorPos_in_doc = ::SendMessage(hScintilla, SCI_GETCURRENTPOS, 0, 0);
	//��ȡ������ڵ��е����
	Sci_Position linePos = SendMessage(hScintilla, SCI_LINEFROMPOSITION, cursorPos_in_doc, 0);
	//��ȡ���е���ʼ��
	Sci_Position lineStartPos_in_doc = SendMessage(hScintilla, SCI_POSITIONFROMLINE, linePos, 0);
	//���н�����
    Sci_Position lineEndPos_in_doc = SendMessage(hScintilla, SCI_GETLINEENDPOSITION, linePos, 0);
	
	//��ȡ�����ı�
	std::string line_text = GetTextByRange(hScintilla , lineStartPos_in_doc , lineEndPos_in_doc);
	if(line_text.empty())
		return;
	
	//���� ���뿪ʼλ��(�м�)
	Sci_Position code_start_pos_in_line = FindCodeLineStartPos(line_text);
	
	//�����У����뿪ʼ��λ�ã����ĵ���λ��
	Sci_Position code_start_pos_in_doc = lineStartPos_in_doc + code_start_pos_in_line;
	
	//���ǻ�ȡ���е� �������
	std::string relative_identation = line_text.substr(0 , code_start_pos_in_line);
	
	//===
	
	//RemoveTextSlashR(line_text);
	
	//std::string lineText = GetCurrentLineString(hScintilla);
	//ȥ���ַ����е� \r
	//RemoveTextSlashR(lineText);
	
	//���� �ı� ȥ��ǰ�������Ĳ���
	std::string code_line = line_text.substr(code_start_pos_in_line);
	if(code_line.empty())
		return;
	
	auto line_parts = splitText(code_line , ' ');
	//���У��м��飿��һ��ض�����������鶼�ǲ���
	//size_t parts_count = line_parts.size();
	std::string cmd_key = line_parts[0];
	
	//ע���ʱ cmd_key ������ǰ�������ģ���Ϊ�������ڲ��������ı��д���������
	//�������ﵥ��Ϊ cmd_key ����������
	//cmd_key = TrimHeader(cmd_key);
	
	if( ! sence.contains(cmd_key) )
	{
		::MessageBox(NULL, TEXT("sence has no this cmd"), TEXT("wrong"), MB_OK);
		
		std::ofstream file_log2(myPluginPath + "log_cmd.txt");
		file_log2 << cmd_key;
		file_log2.close();
		
		LastSnippetLength = 0;
		LastSnippetName = "";
		return;
	}
	
	//�����滻Ϊ�Ĵ����
	std::string snippetContent = sence[cmd_key]["contentText"];
	
	//ConvertToWindowsLineEndings
	
	//Ϊÿ�� ��ӻ�׼����
	snippetContent = AddBaseIdentation(snippetContent , relative_identation);
	
	//����в���ʹ�ñ�ǣ���� �в����������������ŵ�һ����ֵ���
	//�� content �����е� ������ǣ�����ȡ�����ƣ����滻�� ��Ӧ���ı�ֵ
	if(sence[cmd_key]["hasParam"])
	{
		std::vector<std::string> key_list = sence[cmd_key]["params"];
		//����в������������Ƿ�ָ���˲�������ֹԽ��
		if(line_parts.size() != ( key_list.size() + 1 ) )
		{
			::MessageBox(NULL, TEXT("param count wrong"), TEXT("wrong"), MB_OK);
			return;
		}
		
		//�������� �� ����ֵ �ļ�ֵ��
		std::unordered_map<std::string, std::string> dict;
		for(size_t index = 0 ; index < key_list.size() ; ++index)
		{
			auto one_key = key_list[index];
			auto one_value = line_parts[index + 1];
			dict.emplace(one_key , one_value);
		}
		
		//����
		/*
		std::ofstream file_log3(myPluginPath + "dict_log.txt");
		for(const auto & pair : dict)
		{
			file_log3 << pair.first << '=' << pair.second << '\n';
		}
		file_log3.close();
		*/
		
		//��ʼ�����µ� snippetContent
		snippetContent = ReplaceMarkToValue(snippetContent , dict);
	}
	
	//��ȡ��ǰ���з���ģʽ
	int eolMode = (int)::SendMessage(hScintilla, SCI_GETEOLMODE, 0, 0);
	
	if(eolMode == SC_EOL_CRLF)
	{
		snippetContent = ConvertToWindowsLineEndings(snippetContent);
	}
	
	//ѡ�� �Ӵ��뿪ʼһֱ�� �н���
	::SendMessage(hScintilla, SCI_SETSEL, code_start_pos_in_doc, lineEndPos_in_doc);
	//�滻
	::SendMessage(hScintilla, SCI_REPLACESEL, 0, (LPARAM) snippetContent.c_str() );
	
	LastSnippetName = cmd_key;
	LastSnippetLength = snippetContent.size();
	
	//����ˣ���û�꣬��Ҫ��飬�Ƿ�����ת����
	if(! sence[LastSnippetName]["hasJump"])
		return;
	
	LastSnippetLength += 10;	//Ӧ���� +3�����ϳ����⣬������������10������
	
	//�ڵ�ǰλ�ü���׷����ת������ǵ�
	std::string jump_mark = sence[cmd_key]["cursorMark"];
	std::string jump_mark_end = jump_mark + '@';
	
	::SendMessage(hScintilla, SCI_INSERTTEXT, (WPARAM) - 1, (LPARAM)jump_mark_end.c_str());
	
	//���� snippet ֮�����ǣ���ת�� cmd ��ʼ��
	//SCI_GOTOPOS
	::SendMessage(hScintilla, SCI_GOTOPOS , code_start_pos_in_doc , 0);
	
	//���ڣ����Ǵ��ڲ����� ��ת��
	size_t raletive_jump_pos = FindTwoCharPos(snippetContent , jump_mark);
	size_t jump_pos_in_doc = code_start_pos_in_doc + raletive_jump_pos;
	size_t select_end_pos_in_doc = jump_pos_in_doc + jump_mark.size() ;
	::SendMessage(hScintilla, SCI_SETSEL, jump_pos_in_doc, select_end_pos_in_doc );
	return;
}




/*
�˺���������ʱ����
�û��Ѿ��ڴ�����У�����˵�һ�ζ�λ��
�ڵ�ǰ����λ��������

ע���ʱ�û��Ĺ���Ѿ��� snippet �м䲿���ˡ�
*/
void WhenJump()
{
	api_LoadSence();
	
	if(LastSnippetName.empty())
	{
		::MessageBox(NULL, TEXT("LastSnippetName is empty"), TEXT("wrong"), MB_OK);
		return;
	}
	
	if(! sence.contains(LastSnippetName) )
	{
		std::string msg1 = "No this cmd in Sence og " + current_sence_name;
		::MessageBoxA(NULL, msg1.c_str(), "wrong", MB_OK);
		return;
	}
	
	if(! sence[LastSnippetName].contains("cursorMark") )
	{
		::MessageBox(NULL, TEXT("this cmd has no jump mark define"), TEXT("wrong"), MB_OK);
		return;
	}
	if(! sence[LastSnippetName]["hasJump"])
	{
		::MessageBox(NULL, TEXT("this snippet did dont caontains jump mark"), TEXT("wrong"), MB_OK);
		return;
	}
	
	//������־
	std::string jump_mark = sence[LastSnippetName]["cursorMark"];
	std::string end_mark = jump_mark +  '@';
	
	HWND hScintilla = getCurrentScintillaHandle();
	
	//������ ��������λ�ã�Ȼ������ ��ǰ���λ�� �� ������λ�� �� ��ת����
	//�ѵ�����ѡ����ת��û�ѵ�����ֱ�Ӱѽ�����ѡ�м��ɣ�
	//���Ҵ����������Ҫ�� ��ת��� ���
	Sci_Position cursorPos_in_doc = ::SendMessage(hScintilla, SCI_GETCURRENTPOS, 0, 0);
	Sci_Position maxPos_in_doc = cursorPos_in_doc + LastSnippetLength;
	Sci_Position end_flag_pos = FindJumpMarkPos(hScintilla , cursorPos_in_doc , maxPos_in_doc , end_mark);
	
	if(end_flag_pos == std::string::npos)
	{
		::MessageBox(NULL, TEXT("Didnt't find end mark in this snippet"), TEXT("wrong"), MB_OK);
		return;
	}
	
	//�� ��ת����
	Sci_Position jump_mark_pos = FindJumpMarkPos(hScintilla , cursorPos_in_doc , maxPos_in_doc , jump_mark);
	if(jump_mark_pos == std::string::npos)
	{
		::MessageBox(NULL, TEXT("Didnt't find jump mark in this snippet"), TEXT("wrong"), MB_OK);
		return;
	}
	
	//����ҵ��ľ������շ���
	if(jump_mark_pos == end_flag_pos)
	{
		//::MessageBox(NULL, TEXT("Found End Mark"), TEXT("Info"), MB_OK);
		::SendMessage(hScintilla, SCI_SETSEL, jump_mark_pos, jump_mark_pos +  3 );
		//����˴�snippet������صı��
		LastSnippetName = "";
		LastSnippetLength = 0;
		return;
	}
	
	//::MessageBox(NULL, TEXT("Found next Mark"), TEXT("Info"), MB_OK);
	
	//����ҵ��ģ��������շ��ţ���ѡ����
	::SendMessage(hScintilla, SCI_SETSEL, jump_mark_pos, jump_mark_pos +  2 );
}

void Debug()
{
	std::string ext = api_GetExtName();
	std::ofstream f_file(myPluginPath + "ext_log_debug.txt");
	f_file << ext;
}















