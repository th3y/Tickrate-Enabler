#ifndef __UPDATERATE_PATCHES_H__
#define __UPDATERATE_PATCHES_H__

#include "thirdparty/codepatch/icodepatch.h"
#include "misc_asm.h"
#include "sigs.h"

#if defined (SIG_CBASECLIENT_SETUPDATERATE) && defined (SIG_CGAMECLIENT_SETUPDATERATE)
#define UPDATERATE_PATCHES_AVAILABLE

class UpdateRateLimitPatch : public ICodePatch
{
public:
	UpdateRateLimitPatch(BYTE* engine, const char* name, const char* signature, size_t signatureSize, int patchOffset, BYTE checkByte);
	~UpdateRateLimitPatch();
	void Patch();
	void Unpatch();
	bool IsAlreadyUnlocked() const { return m_bAlreadyUnlocked; }

private:
	BYTE* FindFunction(BYTE* engine, const char* signature, size_t signatureSize);
	ICodePatch* GeneratePatch(BYTE* target);
	bool IsUnlocked(const BYTE* target) const;

	ICodePatch* m_patch;
	bool m_bAlreadyUnlocked;
};

class BaseClientUpdateRatePatch : public UpdateRateLimitPatch
{
public:
	BaseClientUpdateRatePatch(BYTE* engine);
};

class GameClientUpdateRatePatch : public UpdateRateLimitPatch
{
public:
	GameClientUpdateRatePatch(BYTE* engine);
};

#endif

#endif
