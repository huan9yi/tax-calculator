#include <sstream>
#include <unordered_map>

#include "resource.h"

#include "htmlayout.h"
#include "json/json.h"

// External
extern HINSTANCE hInst;
extern HWND hMainWnd;
extern void ToTray();
extern wchar_t* CharToWChar(const char*);
extern wchar_t* UTF8CharToWChar(const char*);
extern char* WCharToChar(const wchar_t*);
extern void Debug(int, HWND=NULL);

// Global Variables
int window_width = 500;
int window_height = 580;
char szTitle[] = "工资计算器"; // The title bar text
char szWindowClass[] = "工资计算器"; // The title bar text
const char *app_name = "TexCalculator-9697DD2A-BB85-40EA-A890-DB956C6259AA"; // unique app name base on GUID
float version = 0.1f;

// Emvironment Variables
static Json::Value cities = NULL; // json城市数据

// Foward declarations
void CheckVersion();
void SetPageData(const char*);
void CalculateSalary(std::unordered_map<char*, float>&);
Json::Value ParseCityFromJsonFile();
std::unordered_map<char*, float> InitialDataMap();

void AppInitial()
{
	// initial ui
	//生成城市下拉列表
	htmlayout::dom::element root = htmlayout::dom::element::root_element(hMainWnd);
	htmlayout::dom::element select = root.get_element_by_id("select_city");
	htmlayout::dom::element child = select.child(2);

	Json::Value cities = ParseCityFromJsonFile();
	Json::Value::Members mem = cities.getMemberNames();
	for (auto iter = mem.begin(); iter != mem.end(); iter++){
		const char *char_city = (*iter).c_str();
		if (strcmp(char_city, "guangzhou") == 0){
			continue;
		}
		wchar_t *wchar_city = UTF8CharToWChar(cities[char_city]["name"].asCString());
		wchar_t *wchar_city_value = CharToWChar(char_city);
		
		htmlayout::dom::element option = htmlayout::dom::element::create("option", wchar_city);
		option.set_attribute("value", wchar_city_value);
		child.append(option);

		delete[] wchar_city;
		delete[] wchar_city_value;
	}

	// 初始化界面数据
	SetPageData("initial");

	// 设置焦点
	htmlayout::dom::element input = root.get_element_by_id("salary");
	input.set_state(STATE_FOCUS);

	// 检查版本更新
	CheckVersion();

	//CreateThread(NULL, 0, TrackThread, NULL, 0, NULL);
}

void CheckVersion()
{	
	//TODO
	//Debug("检测到新版本，请到官网下载(http://www.benbenq.com)");
}

void OnButtonClick(HELEMENT button)
{
	htmlayout::dom::element el = button;
	const wchar_t *str = el.get_attribute("id");
	if (str){
		std::wstring id = str;
		if (id == L"action_minimize_window"){
			ToTray();
		}
		else if (id == L"action_close_window"){
			DestroyWindow(hMainWnd);
		}
		else if (id == L"action_calc"){
			SetPageData("action_calc");
		}
		else if (id == L"action_reset"){
			SetPageData("action_reset");
		}
	}
}

void OnSelectSelectionChanged(HELEMENT select)
{
	htmlayout::dom::element el = select;
	const wchar_t *str = el.get_attribute("id");
	if (str){
		std::wstring id = str;
		if (id == L"select_city"){
			SetPageData("select_city");
		}
	}
}

// 设置页面各项数据
void SetPageData(const char *action)
{
	std::unordered_map<char*, float> map = InitialDataMap();

	htmlayout::dom::element root = htmlayout::dom::element::root_element(hMainWnd);

	// 取得输入的工资
	htmlayout::dom::element value_gross_pay = root.get_element_by_id("salary");
	map["salary"] = _wtof(value_gross_pay.text().c_str());

	// 取得选择的城市，获取相关数据
	htmlayout::dom::element select = root.get_element_by_id("select_city");
	htmlayout::dom::element option = select.find_first("option:checked");
	char *char_city = WCharToChar(option.get_attribute("value"));

	Json::Value cities = ParseCityFromJsonFile();
	Json::Value city_tax = cities[char_city]["tax"];

	// 取数据
	char *vars1[] = { "medicare_plan", "threshold", "min_wage", "insurance_max", "insurance_min", "fund_max" };
	for (char *var : vars1){
		map[var] = city_tax[var].asFloat();
	}

	if (strcmp("action_calc", action) == 0){
		char *vars2[] = { "pension", "medicare", "unemployment_insurance", "fund",
			"pension_firm", "medicare_firm", "unemployment_insurance_firm", "industrial_injury_firm", "maternity_insurance_firm", "fund_firm" };

		for (char *var : vars2){
			htmlayout::dom::element el = root.get_element_by_id(var);
			map[var] = _wtof(el.text().c_str());
		}
	}
	else{
		char *vars3[] = { "pension", "medicare", "unemployment_insurance", "fund",
			"pension_firm", "medicare_firm", "unemployment_insurance_firm", "industrial_injury_firm", "maternity_insurance_firm", "fund_firm" };

		for (char *var : vars3){
			map[var] = city_tax[var].asFloat();
		}
	}

	delete[] char_city;

	// 计算各项数据
	CalculateSalary(map);

	// 设置页面数据
	char *vars4[] = { "real_salary", "pension", "medicare", "unemployment_insurance", "industrial_injury", "maternity_insurance", "fund",
		"pension_firm", "medicare_firm", "unemployment_insurance_firm", "industrial_injury_firm", "maternity_insurance_firm", "fund_firm" };

	for (char *var : vars4){
		htmlayout::dom::element el = root.get_element_by_id(var);
		if (strcmp(var, "real_salary") == 0){
			el.set_value(json::value(int(map[var])));
		}
		else{
			el.set_value(json::value(map[var]));
		}
	}

	std::string str;
	std::stringstream ss;
	char *vars5[] = { "tax", "threshold", "pension_fee", "medicare_fee", "unemployment_insurance_fee", "industrial_injury_fee", "maternity_insurance_fee", "fund_fee", "personal_total_fee", 
		"pension_firm_fee", "medicare_firm_fee", "unemployment_insurance_firm_fee", "industrial_injury_firm_fee", "maternity_insurance_firm_fee", "fund_firm_fee", "firm_total_fee" };

	for (char *var : vars5){
		htmlayout::dom::element el = root.get_element_by_id(var);
		ss << map[var];
		ss >> str;
		el.set_html((unsigned char*)str.c_str(), str.length(), SIH_REPLACE_CONTENT);
		ss.clear();
	}
}

// 计算税后工资
void CalculateSalary(std::unordered_map<char*, float> &map)
{
	// 公式：
	// 五险一金 = 社保缴纳基数 * (养老比例 ＋ 医疗比例 + 失业比例 + 工伤比例 + 生育比例) + 公积金缴纳基数 * 公积金比例
	// 税 = 适用税率 * (税前工资 - 五险一金 - 起征点) - 速算扣除数
	// 税后工资 = 税前工资 - 五险一金 - 税

	// 高于最低工资才计算
	if (map["salary"] <= 0 | (map["min_wage"] > 0 && map["salary"] < map["min_wage"]))
	{
		map["real_salary"] = map["salary"];
	}
	else{
		// 社保缴纳基数
		float insurance_base = 0;
		if (map["salary"] > map["insurance_max"]){
			insurance_base = map["insurance_max"];
		}
		else if (map["salary"] < map["insurance_min"]){
			insurance_base = map["insurance_min"];
		}
		else{
			insurance_base = map["salary"];
		}

		// 公积金缴纳基数
		float fund_base = 0;
		if (map["salary"] > map["fund_max"]){
			fund_base = map["fund_max"];
		}
		else if (map["salary"] < map["min_wage"]){
			fund_base = map["min_wage"];
		}
		else{
			fund_base = map["salary"];
		}

		// 五险一金，个人部分
		float pension_fee = insurance_base * map["pension"] / 100;
		float medicare_fee = insurance_base * map["medicare"] / 100 + map["medicare_plan"];
		float unemployment_insurance_fee = insurance_base * map["unemployment_insurance"] / 100;
		float fund_fee = fund_base * map["fund"] / 100;
		float personal_total_fee = pension_fee + medicare_fee + unemployment_insurance_fee + fund_fee;

		if (map["salary"] - personal_total_fee < 0)
		{
			map["real_salary"] = map["salary"];
		}
		else
		{
			// 五险一金，公司部分
			float pension_firm_fee = insurance_base * map["pension_firm"] / 100;
			float medicare_firm_fee = insurance_base * map["medicare_firm"] / 100;
			float unemployment_insurance_firm_fee = insurance_base * map["unemployment_insurance_firm"] / 100;
			float industrial_injury_firm_fee = insurance_base * map["industrial_injury_firm"] / 100;
			float maternity_insurance_firm_fee = insurance_base * map["maternity_insurance_firm"] / 100;
			float fund_firm_fee = fund_base * map["fund_firm"] / 100;
			float firm_total_fee = pension_firm_fee + medicare_firm_fee + unemployment_insurance_firm_fee + industrial_injury_firm_fee + maternity_insurance_firm_fee + fund_firm_fee;

			// 税
			float tax = 0;
			float salary_after_threshold = map["salary"] - personal_total_fee - map["threshold"];
			if (salary_after_threshold > 0){
				if (salary_after_threshold <= 1500)
				{
					tax = salary_after_threshold * 0.03;
				}
				else if (salary_after_threshold > 1500 && salary_after_threshold <= 4500)
				{
					tax = salary_after_threshold * 0.1 - 105;
				}
				else if (salary_after_threshold > 4500 && salary_after_threshold <= 9000)
				{
					tax = salary_after_threshold * 0.2 - 555;
				}
				else if (salary_after_threshold > 9000 && salary_after_threshold <= 35000)
				{
					tax = salary_after_threshold * 0.25 - 1005;
				}
				else if (salary_after_threshold > 35000 && salary_after_threshold <= 55000)
				{
					tax = salary_after_threshold * 0.3 - 2755;
				}
				else if (salary_after_threshold > 55000 && salary_after_threshold <= 80000)
				{
					tax = salary_after_threshold * 0.35 - 5505;
				}
				else if (salary_after_threshold > 80000)
				{
					tax = salary_after_threshold * 0.45 - 13505;
				}
			}

			// 税后工资
			float real_salary = map["salary"] - personal_total_fee - tax;

			map["pension_fee"] = pension_fee;
			map["medicare_fee"] = medicare_fee;
			map["unemployment_insurance_fee"] = unemployment_insurance_fee;
			map["fund_fee"] = fund_fee;
			map["personal_total_fee"] = personal_total_fee;

			map["pension_firm_fee"] = pension_firm_fee;
			map["medicare_firm_fee"] = medicare_firm_fee;
			map["unemployment_insurance_firm_fee"] = unemployment_insurance_firm_fee;
			map["industrial_injury_firm_fee"] = industrial_injury_firm_fee;
			map["maternity_insurance_firm_fee"] = maternity_insurance_firm_fee;
			map["fund_firm_fee"] = fund_firm_fee;
			map["firm_total_fee"] = firm_total_fee;

			map["tax"] = tax;
			map["real_salary"] = real_salary;
		}
	}
}

// 从json文件解析各城市的相关数据
Json::Value ParseCityFromJsonFile()
{
	if (cities == NULL){
		// 加载json_city.txt文件
		HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(IDR_TXT_CITY), "txt");
		if (NULL == hRsrc)
			return NULL;

		DWORD dwSize = SizeofResource(NULL, hRsrc);
		if (0 == dwSize)
			return NULL;

		HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
		if (NULL == hGlobal)
			return NULL;

		// 返回文件内容
		LPVOID pBuffer = LockResource(hGlobal);
		if (NULL == pBuffer)
			return NULL;

		const char *data = static_cast<const char*>(pBuffer);
		char *buffer = new char[dwSize + 1];
		memcpy(buffer, data, dwSize);
		buffer[dwSize] = 0; // NULL terminator

		Json::Reader reader;
		Json::Value value;
		if (reader.parse(buffer, value)){
			cities = value;
		}

		delete[] buffer;
	}

	return cities;
}

// 页面上的各项数据
std::unordered_map<char*, float> InitialDataMap()
{
	std::unordered_map<char*, float> map = {
		// 直接获取
		{ "salary", 0 }, { "pension", 0 }, { "medicare", 0 }, { "medicare_plan", 0 }, { "unemployment_insurance", 0 }, { "industrial_injury", 0 }, { "maternity_insurance", 0 }, { "fund", 0 }, 
		{ "pension_firm", 0 }, { "medicare_firm", 0 }, { "unemployment_insurance_firm", 0 }, { "industrial_injury_firm", 0 }, { "maternity_insurance_firm", 0 }, { "fund_firm", 0 }, 
		{ "threshold", 0 }, { "min_wage", 0 }, { "insurance_max", 0 }, { "insurance_min", 0 }, { "fund_max", 0 }, 
		
		// 需要计算
		{ "pension_fee", 0 }, { "medicare_fee", 0 }, { "unemployment_insurance_fee", 0 }, { "industrial_injury_fee", 0 }, { "maternity_insurance_fee", 0 }, { "fund_fee", 0 }, { "personal_total_fee", 0 }, 
		{ "pension_firm_fee", 0 }, { "medicare_firm_fee", 0 }, { "unemployment_insurance_firm_fee", 0 }, { "industrial_injury_firm_fee", 0 }, { "maternity_insurance_firm_fee", 0 }, { "fund_firm_fee", 0 }, { "firm_total_fee", 0 }, 
		{ "tax", 0 }, { "real_salary", 0 }
	};

	return map;
}

DWORD WINAPI TrackThread(IN PVOID param)
{
	HINSTANCE hDllInst = LoadLibrary("Tracker.dll");
	if (hDllInst){
		typedef void(WINAPI *MYFUNC)();
		MYFUNC trackFuntion = NULL;
		trackFuntion = (MYFUNC)GetProcAddress(hDllInst, "doPage");
		if (trackFuntion){
			trackFuntion();
		}
		FreeLibrary(hDllInst);
	}
	return 0;
}