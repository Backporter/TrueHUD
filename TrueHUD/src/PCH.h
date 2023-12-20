#pragma once

#pragma warning(push)
#include "../../../CSEL/source/RE/Skyrim.h"
#include "../../../CSEL/source/REL/Relocation.h"
#include "../../../CSEL/EXTERNAL/SKSE/SKSE.h"
#include "../../../OrbisUtil/Third-Party/brofield/1.0/SimpleIni.h"
#include "../../../OrbisUtil/include/Logger.h"
#include "../../../OrbisUtil/include/FileSystem.h"
#include "ModAPI.h"

#ifdef NDEBUG
#else
#endif
#pragma warning(pop)

#define MAP_CONTAINS(map, data) map.find(find) != map.end()

#define DLLEXPORT __declspec(dllexport)
#define RELOCATION_OFFSET(SE, AE) 0 // REL::VariantOffset(SE, AE, 0).offset()

// #include "Plugin.h"
