#include "updaterate_patches.h"

#if defined (UPDATERATE_PATCHES_AVAILABLE)

#include "thirdparty/memutils.h"
#include "patchexceptions.h"
#include "basicbinpatch.h"

UpdateRateLimitPatch::UpdateRateLimitPatch(BYTE* engine, const char* name, const char* signature, size_t signatureSize, int patchOffset, BYTE checkByte)
	: m_patch(NULL), m_bAlreadyUnlocked(false)
{
	BYTE* function = FindFunction(engine, signature, signatureSize);
	if (!function)
	{
		throw PatchException(name);
	}

	BYTE* target = function + patchOffset;
	if (IsUnlocked(target))
	{
		m_bAlreadyUnlocked = true;
		return;
	}

	if (target[0] != checkByte)
	{
		throw PatchException(name);
	}

	m_patch = GeneratePatch(target);
}

UpdateRateLimitPatch::~UpdateRateLimitPatch()
{
	if (m_patch != NULL) delete m_patch;
}

void UpdateRateLimitPatch::Patch()
{
	if (m_patch != NULL) m_patch->Patch();
}

void UpdateRateLimitPatch::Unpatch()
{
	if (m_patch != NULL) m_patch->Unpatch();
}

BYTE* UpdateRateLimitPatch::FindFunction(BYTE* engine, const char* signature, size_t signatureSize)
{
#if defined (_LINUX)
	return (BYTE*)g_MemUtils.SimpleResolve(engine, signature);
#elif defined (_WIN32)
	return (BYTE*)g_MemUtils.FindLibPattern(engine, signature, signatureSize);
#endif
}

#if defined (_WIN32)
ICodePatch* UpdateRateLimitPatch::GeneratePatch(BYTE* target)
{
	const BYTE replacement[] = { JMP_8_OPCODE };
	return new BasicStaticBinPatch<sizeof(replacement)>(target, replacement);
}

bool UpdateRateLimitPatch::IsUnlocked(const BYTE* target) const
{
	return target[0] == JMP_8_OPCODE;
}
#elif defined (_LINUX)
ICodePatch* UpdateRateLimitPatch::GeneratePatch(BYTE* target)
{
	if (target[0] == CMOVLE_PREFIX)
	{
		const BYTE replacement[] = { MOV_R32_RM32_OPCODE, MODRM_REG_EDX_EAX, NOP_1[0] };
		return new BasicStaticBinPatch<sizeof(replacement)>(target, replacement);
	}

	return new BasicStaticBinPatch<sizeof(NOP_2)>(target, NOP_2);
}

bool UpdateRateLimitPatch::IsUnlocked(const BYTE* target) const
{
	bool movUnlocked = target[0] == MOV_R32_RM32_OPCODE && target[1] == MODRM_REG_EDX_EAX;
	bool nopUnlocked = target[0] == NOP_2[0] && target[1] == NOP_2[1];
	return movUnlocked || nopUnlocked;
}
#endif

BaseClientUpdateRatePatch::BaseClientUpdateRatePatch(BYTE* engine)
	: UpdateRateLimitPatch(engine, "CBaseClient::SetUpdateRate", SIG_CBASECLIENT_SETUPDATERATE,
#if defined (_WIN32)
		SIG_CBASECLIENT_SETUPDATERATE_SIZE,
#else
		0,
#endif
		CBASECLIENT_UPDATERATE_PATCH_OFFSET, CBASECLIENT_UPDATERATE_CHECK_BYTE)
{
}

GameClientUpdateRatePatch::GameClientUpdateRatePatch(BYTE* engine)
	: UpdateRateLimitPatch(engine, "CGameClient::SetUpdateRate", SIG_CGAMECLIENT_SETUPDATERATE,
#if defined (_WIN32)
		SIG_CGAMECLIENT_SETUPDATERATE_SIZE,
#else
		0,
#endif
		CGAMECLIENT_UPDATERATE_PATCH_OFFSET, CGAMECLIENT_UPDATERATE_CHECK_BYTE)
{
}

#endif
