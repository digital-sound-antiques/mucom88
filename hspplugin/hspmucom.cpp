
//
//		HSP MUCOM88 support DLL ( for ver3.0 )
//				onion software/onitama 2018/11
//

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <rpc.h>

#include "hspdll.h"
#include "../src/cmucom.h"
#include "../src/wavout.h"

#define MUCOM_USE_UUID

#ifdef MUCOM_USE_UUID
#pragma comment(lib,"rpcrt4.lib")
#endif

static void Alertf(char *format, ...)
{
	char textbf[1024];
	va_list args;
	va_start(args, format);
	vsprintf(textbf, format, args);
	va_end(args);
	MessageBox(NULL, textbf, "error", MB_ICONINFORMATION | MB_OK);
}

static void GetUUID(char *res)
{
#ifdef MUCOM_USE_UUID
	UUID newId;
	RPC_CSTR p;
	UuidCreate(&newId);
	UuidToString(&newId, &p);
	strcpy(res, (char *)p);
	RpcStringFree(&p);
#else
	*res = 0;
#endif
}

/*------------------------------------------------------------*/

static CMucom *mucom = NULL;

static void mucom_term(void)
{
	if (mucom == NULL) return;

	mucom->Stop(1);
	delete mucom;
	mucom = NULL;
}

static int mucom_init(void)
{
	mucom_term();

	mucom = new CMucom;
	return 0;
}

/*------------------------------------------------------------*/

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	if ( fdwReason==DLL_PROCESS_ATTACH ) {
		mucom = NULL;
	}
	if ( fdwReason==DLL_PROCESS_DETACH ) {
		mucom_term();
	}
	return TRUE ;
}

char *getvptr( HSPEXINFO *hei, PVal **pval, int *size )
{
	//		変数ポインタを得る
	//
	APTR aptr;
	PDAT *pdat;
	HspVarProc *proc;
	aptr = hei->HspFunc_prm_getva( pval );
	proc = hei->HspFunc_getproc( (*pval)->flag );
	(*pval)->offset = aptr;
	pdat = proc->GetPtr( *pval );
	return (char *)proc->GetBlockSize( *pval, pdat, size );
}

/*
static int valsize( PVAL2 *pv )
{
	//		calc object memory used size
	//			result : size(byte)
	int i,j;
	int vm=1;
	i=1;
	while(1) {
		j=pv->len[i];if (j==0) break;
		vm*=j;i++;if (i==5) break;
	}
	return vm<<2;
}
*/
/*------------------------------------------------------------*/

EXPORT BOOL WINAPI mucominit( int p1, int p2, int p3, int p4 )
{
	//	DLL mucominit hwnd, option (type$00)
	//
	int res;
	res = mucom_init();
	mucom->Init((HWND)p1,p2);
	return res;
}


EXPORT BOOL WINAPI mucombye(int p1, int p2, int p3, int p4)
{
	//	DLL mucombye (type$00)
	//
	mucom_term();
	return 0;
}


EXPORT BOOL WINAPI mucomreset(int p1, int p2, int p3, int p4)
{
	//	DLL mucomreset option (type$00)
	//
	if (mucom) {
		mucom->Reset(p1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomplay(int p1, int p2, int p3, int p4)
{
	//	DLL mucomplay id,flag (type$00)
	//
	if (mucom) {
		mucom->Play(p1,p2==0);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomstop(int p1, int p2, int p3, int p4)
{
	//	DLL mucomstop mode (type$00)
	//
	if (mucom) {
		if (p1 & 0x1000) {
			mucom->Restart();
		}
		else {
			mucom->Stop(p1);
		}
	}
	return 0;
}

EXPORT BOOL WINAPI mucomplaymemory(int p1, int p2, int p3, int p4)
{
	//	DLL mucomplay id (type$00)
	//
	if (mucom) {
		mucom->PlayMemory();
	}
	return 0;
}


// 曲データをコンパイル（MUB出力)
EXPORT BOOL WINAPI mucomcompilefile(const char *file, const char *mubname)
{
	if (mucom) {
		mucom->CompileFile(file, mubname);
	}
	return 0;
}

// 曲データをコンパイル（ファイル出力なし)
EXPORT BOOL WINAPI mucomcompilemem(const char* file)
{
	if (!mucom) return 0;

	mucom->CompileMemory(file);
	return 0;
}

// バッファを準備
EXPORT BOOL WINAPI mucompreparebuf() {
	if (!mucom) return 0;
	mucom->PrintInfoBuffer();
	return 0;
}

// メッセージバッファサイズ取得
EXPORT int WINAPI mucomgetmsgsize() {
	if (!mucom) return 0;
	return mucom->GetMessageBufferSize() + 2;
}

// メッセージバッファ取得
EXPORT int WINAPI mucomgetmsg(char *message, int length) {
	if (!mucom || length == 0) return 0;
	const char* p = mucom->GetMessageBuffer();
	int l = mucom->GetMessageBufferSize();

	strcpy(message, p);
	return 0;
}

// メモリを設定
EXPORT BOOL WINAPI mucomsetmemory(unsigned char* data, int select, int bank, int address, int length)
{
	if (!mucom) return -1;
	switch (select) {
		case 0:
			mucom->SetMainMemory(data, address, length);
		break;
		case 2:
			mucom->SetExtMemory(data, bank, address, length);
			break;
	}
	return 0;
}


// VMメモリを取得する
EXPORT BOOL WINAPI mucomgetmemory(unsigned char *data, int address, int length)
{
	if (!mucom) return -1;
	mucom->GetMemory(data, address, length);
	return 0;
}

// メインメモリを取得する
EXPORT BOOL WINAPI mucomgetmainmemory(unsigned char* data, int address, int length)
{
	if (!mucom) return -1;
	mucom->GetMainMemory(data, address, length);
	return 0;
}


// 拡張メモリを取得する
EXPORT BOOL WINAPI mucomgetextmemory(unsigned char* data, int bank, int address, int length)
{
	if (!mucom) return -1;
	mucom->GetExtMemory(data, bank, address, length);
	return 0;
}

// OPNAレジスタを取得する
EXPORT BOOL WINAPI mucomgetfmmem(unsigned char* data, int address, int length)
{
	if (!mucom) return -1;
	mucom->GetFMRegMemory(data, address, length);
	return 0;
}

// チャンネルワークを取得する
EXPORT BOOL WINAPI mucomgetchwork(int channel, PCHDATA *chwork)
{
	if (!mucom) return -1;
	mucom->GetChannelData(channel, chwork);
	return 0;
}

EXPORT int WINAPI mucomgetstatus(int p1) {
	if (!mucom) return 0;
	return mucom->GetStatus(p1);
}

EXPORT int WINAPI mucomgetfmreg(int reg) {
	if (!mucom) return 0;
	return mucom->FMRegDataGet(reg);
}

EXPORT int WINAPI mucomoutfmreg(int reg, int data) {
	if (!mucom) return 0;
	mucom->FMRegDataOut(reg, data);
	return 0;
}

EXPORT int WINAPI mucomsetmute(int ch, int data) {
	if (!mucom) return 0;
	mucom->SetChMute(ch, data != 0 ? true : false);
	return 0;
}

EXPORT int WINAPI mucomgetmute(int ch) {
	if (!mucom) return 0;
	bool result = mucom->GetChMute(ch);
	return result ? 1 : 0;
}

EXPORT int WINAPI mucompeek(int adr) {
	if (!mucom) return 0;
	return mucom->Peek(adr);
}

EXPORT int WINAPI mucompeekw(int adr) {
	if (!mucom) return 0;
	return mucom->Peekw(adr);
}

EXPORT int WINAPI mucompoke(int adr, int data) {
	if (!mucom) return 0;
	mucom->Poke(adr, data);
	return 0;
}

EXPORT int WINAPI mucompokew(int adr, int data) {
	if (!mucom) return 0;
	mucom->Pokew(adr, data);
	return 0;
}

/*------------------------------------------------------------*/
// デバッグ

EXPORT BOOL WINAPI mucomenablebreakpoint(int adrs)
{
	if (!mucom) return -1;
	mucom->EnableBreakPoint(adrs);
	return 0;
}

EXPORT BOOL WINAPI mucomdisablebreakpoint()
{
	if (!mucom) return -1;
	mucom->DisableBreakPoint();
	return 0;
}

EXPORT BOOL WINAPI mucomdebuggo()
{
	if (!mucom) return -1;
	mucom->DebugRun();
	return 0;
}

EXPORT BOOL WINAPI mucomdebuginstexec()
{
	if (!mucom) return -1;
	mucom->DebugInstExec();
	return 0;
}

EXPORT BOOL WINAPI mucomdebugpause()
{
	if (!mucom) return -1;
	mucom->DebugPause();
	return 0;
}


EXPORT BOOL WINAPI mucomgetregset(RegSet* reg)
{
	if (!mucom) return -1;
	mucom->GetRegSet(reg);
	return 0;
}


/*------------------------------------------------------------*/
// 基本機能関連

EXPORT BOOL WINAPI mucomfade(int p1, int p2, int p3, int p4)
{
	//	DLL mucomfade speed (type$00)
	//
	if (mucom) {
		mucom->Fade();
	}
	return 0;
}


EXPORT BOOL WINAPI mucomloadpcm(void *bmscr, char *p1, int p2, int p3)
{
	//	DLL mucomloadpcm "filename" (type$16)
	//
	if (mucom) {
		int res;
		if (*p1 == 0) {
			res = mucom->LoadPCM();
		}
		else {
			res = mucom->LoadPCM(p1);
		}
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomloadvoice(void *bmscr, char *p1, int p2, int p3)
{
	//	DLL mucomloadvoice "filename" (type$16)
	//
	if (mucom) {
		int res;
		res = mucom->LoadFMVoice(p1);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomload(void *bmscr, char *p1, int p2, int p3)
{
	//	DLL mucomload "filename",num (type$16)
	//
	if (mucom) {
		int res;
		res = mucom->LoadMusic(p1, p2);
		if (res) return -1;
		mucom->LoadTagFromMusic(p2);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomres(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomres var (type$202)
	//
	PVal *pv;
	APTR ap;
	const char *res;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	res = "";
	if (mucom) {
		res = mucom->GetMessageBuffer();
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomstat(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomstat var,option (type$202)
	//
	PVal *pv;
	APTR ap;
	int ep1;
	int res;
	ap = hei->HspFunc_prm_getva(&pv);	// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	res = 0;
	if (mucom) {
		res = mucom->GetStatus(ep1);
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_INT, &res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomcomp(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomcomp "mmlfile","outfile",option (type$202)
	//
	int ep1;
	int res;
	char mmlfile[_MAX_PATH];
	char outfile[_MAX_PATH];
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy( mmlfile,p, _MAX_PATH );
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	strncpy(outfile, p, _MAX_PATH);
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ3:数値
	res = 0;
	if (mucom) {
		res = mucom->CompileFile(mmlfile, outfile, ep1);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucommml(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucommml mmldata_var,option (type$202)
	//
	PVal *pv;
	APTR ap;
	int ep1;
	int res;
	char *p;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	res = 0;
	p = (char *)pv->pt;

	if (mucom) {
		res = mucom->CompileMem(p, ep1);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomtag(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomtag var,"tagname" (type$202)
	//
	PVal *pv;
	APTR ap;
	const char *res;
	char *p;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	p = hei->HspFunc_prm_getds("");			// パラメータ2:文字列
	res = "";
	if (mucom) {
		if (*p == 0) {
			res = mucom->GetInfoBuffer();
		}
		else {
			res = mucom->GetInfoBufferByName(p);
		}
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomloadtag(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomloadtag var,"mmlfile" (type$202)
	//
	int res;
	const char *p;
	PVal *pv;
	APTR ap;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	if (mucom) {
		res = mucom->ProcessFile(p);
		if (res == 0) {
			p = mucom->GetInfoBuffer();
		}
		else {
			p = "";
		}
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, p);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomcnvpcm(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomcnvpcm "WAVfile","ADPCMfile" (type$202)
	//
	int res;
	char wavfile[_MAX_PATH];
	char outfile[_MAX_PATH];
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy(wavfile, p, _MAX_PATH);
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	strncpy(outfile, p, _MAX_PATH);
	if (mucom) {
		res = mucom->ConvertADPCM(wavfile, outfile);
		if (res<0) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI getuuid(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL getuuid var (type$202)
	//
	PVal *pv;
	APTR ap;
	char uuid[64];
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	GetUUID(uuid);
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, uuid);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomsetuuid(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetuuid "UUID" (type$202)
	//
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	if (mucom) {
		mucom->SetUUID(p);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomsetoption(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetoption option, mode (type$202)
	//		mode : 0=set/1=add/2=sub
	//
	int ep1, ep2;
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	ep2 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	if (mucom) {
		mucom->SetVMOption(ep1, ep2);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomsetfastfw(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetfastfw speed (type$202)
	//
	int ep1;
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	if (mucom) {
		mucom->SetFastFW(ep1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomsetvolume(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetvolume fmvol, ssgvol (type$202)
	//
	int ep1, ep2;
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	ep2 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	if (mucom) {
		mucom->SetVolume(ep1, ep2);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomgetchdata(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomgetchdata var, ch (type$202)
	//
	int ep1,ep2;
	PVal *pv;
	APTR ap;
	PCHDATA *pt;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	if (pv->len[1] < 64) return -1;
	if (mucom) {
		pt = (PCHDATA *)pv->pt;
		ep2 = mucom->GetChannelData( ep1,pt );
		if (ep2 != 0) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomrecord(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomrecord "filename",time (type$202)
	//	(wav録音機能は通常起動すると任意に実行できないため機能保留中)
	//
	int ep1;
	char *p;
	char outfile[_MAX_PATH];
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy(outfile, p, _MAX_PATH);
	ep1 = hei->HspFunc_prm_getdi(90);	// パラメータ1:数値
	if (mucom) {
		RecordWave(mucom, outfile, MUCOM_AUDIO_RATE, ep1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomplg_init(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomplg_init "filename" (type$202)
	//	MUCOM88プラグインの追加と初期化
	//
	int res;
	int ep1;
	char *p;
	p = hei->HspFunc_prm_gets();		// パラメータ1:文字列
	ep1 = hei->HspFunc_prm_getdi(0);	// パラメータ2:数値

	if (mucom) {
		res = mucom->AddPlugins(p, ep1);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomplg_notice(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomplg_notice p1,p2,p3 (type$202)
	//	MUCOM88プラグインの通知
	//
	int ep1,ep2,ep3;
	ep1 = hei->HspFunc_prm_getdi(0);	// パラメータ1:数値
	ep2 = hei->HspFunc_prm_getdi(0);	// パラメータ2:数値
	ep3 = hei->HspFunc_prm_getdi(0);	// パラメータ3:数値

	if (mucom) {
		mucom->NoticePlugins(ep1, (void *)ep2,(void *)ep3);
	}
	return 0;
}


//--------------------------------------------------------------------------------------

EXPORT BOOL WINAPI mucomedit_reset(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_reset "MML", option (type$202)
	//	MMLエディタサービスのリセット
	//
	int ep1;
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	if (mucom) {
		mucom->EditorReset( p, ep1 );
	}
	return 0;
}


EXPORT BOOL WINAPI mucomedit_setfile(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_setfile "filename", "pathname",session (type$202)
	//	MMLエディタファイル名の設定
	//	(session=1の場合は音色ファイルを含めて初期化する)
	//
	char *p;
	char filename[_MAX_PATH];
	char pathname[_MAX_PATH];
	int ep1;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy(filename, p, _MAX_PATH);
	p = hei->HspFunc_prm_getds("");			// パラメータ2:文字列
	strncpy(pathname, p, _MAX_PATH);
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ3:数値
	if (mucom) {
		mucom->EditorSetFileName(filename, pathname,ep1!=0);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomedit_getstat(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_getstat var, prmid (type$202)
	//	MMLエディタのステータス取得
	//	(prmid:0=status/1=notice/2=option/3=update check)
	//
	PVal *pv;
	APTR ap;
	int ep1;
	int res;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	res = 0;
	if (mucom) {
		switch (ep1) {
		case 0:
			res = mucom->GetEditorStatus();
			break;
		case 1:
			res = mucom->GetEditorNotice();
			break;
		case 2:
			res = mucom->GetEditorOption();
			break;
		case 3:
			res = mucom->CheckEditorUpdate();
			break;
		}
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_INT, &res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomedit_update(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_update var (type$202)
	//	MMLエディタのテキスト更新(varは配列ではない単一の変数)
	//
	PVal *pv;
	APTR ap;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	if (mucom) {
		mucom->UpdateEditorMML( pv->pt );
	}
	return 0;
}


EXPORT BOOL WINAPI mucomedit_getline(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_getline var, pos (type$202)
	//	MMLエディタのpos->line取得
	//
	PVal *pv;
	APTR ap;
	int ep1;
	int res;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	res = 0;
	if (mucom) {
		res = mucom->GetEditorPosToLine(ep1);
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_INT, &res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomedit_save(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_save "filename" (type$202)
	//	MMLエディタの保存実行
	//	(filenameが""の場合はデフォルト名、指定された場合はその名前で保存する)
	//
	int res;
	char *p;
	p = hei->HspFunc_prm_getds("");			// パラメータ1:文字列

	if (mucom) {
		res = mucom->SaveEditorMML(p);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomedit_getreq(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_getreq var (type$202)
	//	MMLエディタのリクエストMML文字列を取得
	//
	PVal *pv;
	APTR ap;
	const char *p;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	if (mucom) {
		p = mucom->GetRequestMML();
		if (p == NULL) return -1;
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, p);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomedit_proc(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_proc var (type$202)
	//	MMLエディタの定期処理(オートセーブなど)
	//	varの変数にnotice情報を代入する
	//
	PVal *pv;
	APTR ap;
	int res;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	res = 0;
	if (mucom) {
		res = mucom->UpdateEditor();
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_INT, &res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomedit_flush(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomedit_flush (type$202)
	//	MMLエディタの一時ファイルをすべて破棄する
	//
	int res;
	if (mucom) {
		res = mucom->SaveFMVoice(false);
		if (res) return -1;
	}
	return 0;
}


//--------------------------------------------------------------------------------------


EXPORT BOOL WINAPI mucomgetdriver(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomgetdriver var,"filename",option (type$202)
	//		"filename"の#driverオプションを得る
	//		varに結果が入る(MUCOM_DRIVER_*)
	//		optionが0の時は.muc、1の時は.mubを読み込む
	//
	PVal *pv;
	APTR ap;
	int res;
	int ep1;
	char *p;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ3:数値
	res = 0;
	if (mucom) {
		if (ep1 == 0) {
			res = mucom->GetDriverMode(p);
		}
		else {
			res = mucom->GetDriverModeMUB(p);
		}
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_INT, &res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomsetdriver(int p1, int p2, int p3, int p4)
{
	//	DLL mucomsetdriver option (type$00)
	//		mucomのドライバ動作モードを指定する、optionに MUCOM_DRIVER_* を指定する
	//		(実際の反映はReset実行後になる)
	//
	if (mucom) {
		mucom->SetDriverMode(p1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomdumpvoice(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomdumpvoice var (type$202)
	//	ボイスダンプ文字列を取得
	//
	PVal *pv;
	APTR ap;
	char *p;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	if (mucom) {
		p = mucom->DumpFMVoiceAll();
		if (p == NULL) return -1;
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, p);	// 変数に値を代入
	return 0;
}


