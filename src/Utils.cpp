#include "Utils.h"

namespace Utils
{
	RE::FormID getTrimmedFormID(const RE::TESForm* form)
	{
		if (!form)
		{
			return 0;
		}

		const auto array = form->sourceFiles.array;
		if (!array || array->empty())
		{
			return 0;
		}

		RE::FormID formID = form->GetFormID() & 0xFFFFFF; // remove file index -> 0x00XXXXXX
		if (array->front()->IsLight())
		{
			formID = formID & 0xFFF; // remove ESL index -> 0x00000XXX
		}

		return formID;
	}


	std::string getModName(const RE::TESForm* form)
	{
		if (!form)
		{
			return "";
		}

		const auto array = form->sourceFiles.array;
		if (!array || array->empty())
		{
			return "";
		}

		const auto file = array->front();
		std::string_view filename = file ? file->GetFilename() : "";

		return filename.data();
	}

	void loadINIStringSetting(const CSimpleIniA& a_ini, const char* a_sectionName, const char* a_settingName, std::string& a_setting)
	{
		const char* found = nullptr;
		found = a_ini.GetValue(a_sectionName, a_settingName);
		if (found)
		{
			a_setting = a_ini.GetValue(a_sectionName, a_settingName);
		}
	}

}