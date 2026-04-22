#include "NormaliseRE.h"

static void PrintNormalizedRE(const unordered_map<string, string>& mp, const vector<Rule>& vRules) {
	cout << "========= [Normalized Definitions] =========" << endl;
	for (const auto& def : mp) {
		cout << def.first << " => " << def.second << endl;
	}

	cout << endl << "========= [Normalized Rules] =========" << endl;
	int idx = 0;
	for (const auto& rule : vRules) {
		cout << "Rule " << idx++ << ":" << endl;
		cout << "Pattern : " << rule.pattern << endl;
		cout << "Action  : " << rule.actions << endl;
		cout << "----------------------------------------------------" << endl;
	}
}

//��������{}�еĽ����滻
void NormalizeRE::HandleBrace(string& s, unordered_map<string, string>& mp)
{
	string replace_item = "";
	bool brace = false;
	string res = "";
	for (int i = 0; i < s.size(); i++)
	{
		// ����ת��
		if (s[i] == '{' && ((i > 0 && s[i - 1] != '\\') || i == 0))
		{
			brace = true;
			continue;
		}
		// ����/}��ǰ�汻ȥ����ʱת�����
		else if (s[i] == '}' && (i > 0 && s[i - 1] != '\\'))
		{
			brace = false;
			// �滻replace_item
			auto it = mp.find(replace_item);
			if (it == mp.end())
			{
				cout << "ERR : replace_item dosen't exist" << endl
					<< "replace_item = " << replace_item << endl
					<< "cur_item = " << s << endl;
				exit(1);
			}
			res += mp[replace_item];
			replace_item.clear();
		}
		else if (!brace)// ���ڻ�������
			res += s[i];
		else// ���ǻ����ţ����ڻ�������
			replace_item += s[i];
	}
	s = move(res);
}

//��s�е���ʽת����������ת���ַ�,���ڴ���[ \t\v\n\f]����
void NormalizeRE::HandleLexRC(string& s)
{
	if (s.find('\\') == string::npos)
		return;

	string ans = "";
	bool flag = false;
	for (auto& c : s) {
		if (flag) {
			switch (c) {
			case 'n': ans += '\n'; break;
			case 't': ans += '\t'; break;
			case 'v': ans += '\v'; break;
			case 'f': ans += '\f'; break;
			case '\\': ans += '\\'; break;
			default: ans += c; break;  // ����δ֪ת������� \x��
			}
			flag = false;
		}
		else {
			if (c == '\\') {
				flag = true;
			}
			else {
				ans += c;
			}
		}
	}
	s = std::move(ans);
}

// ��ȡ��[a-z]���ַ����� flag��ʾ�Ƿ���^
void NormalizeRE::getSet(unordered_set<char>& charSet, string& s, bool flag)
{
	unordered_set<char> newset;
	int i = 0;
	//��rec�е���ʽת����������ת���ַ�
	HandleLexRC(s);
	for (i = 0; i < s.size(); i++)
	{
		char c = s[i];
		if (c == '-' && i > 0 && i < s.size() - 1)
		{
			char first = s[i - 1];
			char last = s[i + 1];
			// 按 ASCII 顺序添加从 first 到 last 的所有字符
			for (char ch = first; ch <= last; ++ch) {
				newset.insert(ch);
			}
			i += 1;
		}
		else
			newset.insert(c);
	}

	//����flag���ַ�����newset
	if (flag)
	{
		//��^
		// 检查补集是否包含大部分字符，如果是，使用 . 代替
		string common_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#%'()*+,-./:;<=>?[\\]^_`{|}~ &";
		int common_count = 0;
		for (char ch : common_chars) {
			if (newset.find(ch) == newset.end()) 
				common_count++;
		}
		
		// 如果补集包含大部分常见字符，使用 . 代替
		if (common_count > common_chars.size() * 0.8) {
			charSet.clear();
			charSet.insert('.');
		}
		else {
			for (char ch : common_chars) {
				if (newset.find(ch) == newset.end()) 
					charSet.insert(ch);
			}
		}
	}
	else
		charSet = move(newset);
}

// ����������[]���������е�����ת��Ϊ(a|b|c)
void NormalizeRE::HandleBrackets(string& s)
{
	string replace_item = "";
	bool braket = false;
	string res = "";
	for (int i = 0; i < s.size(); i++)
	{
		//��ת�� \[
		if (s[i] == '[' && (i == 0 || (i > 0 && s[i - 1] != '\\')))
		{
			braket = true;
			continue;
		}
		//��ת�� \]
		else if (s[i] == ']' && (i > 0 && s[i - 1] != '\\'))
		{
			cout << "[replace_item] : " << replace_item << endl;
			braket = false;
			//�����滻
			unordered_set<char> lcharset;//�ֲ�charset
			if (replace_item[0] == '^')
			{
				string charStr = replace_item.substr(1, replace_item.size() - 1);
				getSet(lcharset, charStr, true);
			}
			else
				getSet(lcharset, replace_item, false);
			res += "(";
			for (auto& c : lcharset)
			{
				// �ַ�c��lexԤ���֣�����ת��
				if (lex_RC.find(c) != lex_RC.end())
					res += '\\';
				res += c;
				res += '|';
			}
			res.pop_back();
			res += ')';
			replace_item.clear();
		}
		else if (braket)//��[]��
			replace_item += s[i];
		else
			res += s[i];
	}
	s = move(res);
}


// ��ȥ�������������е������ַ�����ת��
void NormalizeRE::HandleQuote(string& s)
{
	bool quote = false;
	string res = "";
	for (int i = 0; i < s.length(); i++)
	{
		//��ǰ�ַ���",����i==0����\""
		if (s[i] == '"')
		{
			if (!quote)
			{
				quote = true;
				continue;
			}
			else 
			{
				quote = false;
				continue;
			}
		}
		// �������ڲ�����lexREԤ���ַ���ʹ��\\����ת��
		else if (quote && lex_RC.find(s[i]) != lex_RC.end())
			res += '\\';
		res += s[i];
	}
	//����
	s = move(res);
}

//�� . �滻Ϊ[^ \n\v\f\t]��
void NormalizeRE::HandleDot(string& s)
{
	if (s.find('.') == string::npos)
		return;
	string ans = "";
	int n = s.size();
	for (int i = 0; i < n; i++) {
		if (s[i] == '.' && (i == 0 || (i != 0 && s[i - 1] != '\\'))) {
			ans += '(';
			unordered_set<char> charset;
			string replace_item = " \t\v\n\f"; //[^ \n\v\f\t]
			// ��ȡ�����ַ�
			getSet(charset, replace_item, true);
			for (auto c : charset) {
				if (lex_RC.find(c) != lex_RC.end())
					ans += '\\';
				ans += c; 
				ans += '|';
			}
			ans.pop_back();
			ans += ')';
			continue;
		}
		ans += s[i];
	}
	s = move(ans);
}


void NormalizeRE::RENormalization(vector<Rule>& re, unordered_map<string, string>& mp)
{
	for (auto& m : mp) {
		HandleQuote(m.second);
		HandleBrace(m.second, mp);
		HandleBrackets(m.second); 
	}

	int i = 0;
	for (auto& r : re) {
		HandleQuote(r.pattern);
		HandleBrace(r.pattern, mp);
		HandleBrackets(r.pattern);
		HandleDot(r.pattern);
		//HandleLexRC(r.pattern, false);
	}

	PrintNormalizedRE(mp, re);
}