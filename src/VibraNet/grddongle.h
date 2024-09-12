#ifndef GRD_DONGLE_H
#define GRD_DONGLE_H

#include <cstddef>
#include "grdapi.h"

class CGrdDongle
{
		struct InitializeGrdDongle
		{
			InitializeGrdDongle(DWORD dwRemoteMode)
			{
				GrdStartup(dwRemoteMode);
			}
			~InitializeGrdDongle()
			{
				GrdCleanup();
			}
		};

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64)
		struct InitializeDongleNotification
		{
			InitializeDongleNotification()
			{
				GrdInitializeNotificationAPI();
			}
			~InitializeDongleNotification()
			{
				GrdUnInitializeNotificationAPI();
			}
		};
#endif // __WIN32__ || _WIN32 || WIN32 || _WIN64

		// Protected attributes.
	protected:
		BYTE   m_abyGrdContainer[GrdContainerSize];
		HANDLE m_hGrd;
		DWORD  m_dwRemoteMode;
		// parameters for call SetFindMode()
		DWORD  m_dwFlags;                                      // Combination of GrdFM_XXXXX flags
		DWORD  m_dwProg;
		DWORD  m_dwVer;
		DWORD  m_dwMask;
		DWORD  m_dwType;
		DWORD  m_dwModels;                                     // Possible dongle model bits. See GrdFMM_GSX definition
		DWORD  m_dwInterfaces;                                 // Possible dongle interface bits. See GrdFMI_XXX definition
	public:
		// Constructor
		CGrdDongle(
				// parameters for call GrdStartup() & GrdSetFindMode
				DWORD dwRemoteMode = GrdFMR_Local + GrdFMR_Remote,
				// parameters for call SetFindMode()
				DWORD dwFlags      = GrdFM_NProg + GrdFM_Ver,     // Combination of GrdFM_XXXXX flags
				DWORD dwProg       = 0,
				DWORD dwID         = 0,
				DWORD dwSN         = 0,
				DWORD dwVer        = 1,
				DWORD dwMask       = 0,
				DWORD dwType       = 0,
				DWORD dwModels     = GrdFMM_ALL,                  // Possible dongle model bits. See GrdFMM_GSX definition
				DWORD dwInterfaces = GrdFMI_ALL);                 // Possible dongle interface bits. See GrdFMI_XXX definition

		// Destructor
		~CGrdDongle();

		// Create class object. It call GrdStartup, GrdCreateHandle, GrdSetAccessCodes, GrdSetWorkMode, GrdSetFindMode
		// and must by call after constructor and before any first other member function calls
		int Create(
				// parameters for call SetAccessCodes()
				DWORD dwPublic,
				DWORD dwPrivateRD,
				DWORD dwPrivateWR  = 0,
				DWORD dwPrivateMST = 0,
				// parameters for call SetWorkMode()
				DWORD dwFlagsWork  = GrdWM_UAM,                   // combination of GrdWM_XXX flags
				DWORD dwFlagsMode  = GrdWMFM_DriverAuto);         // combination of GrdWMFM_XXX flags

		// Get current object handle. Null if it not initialized by call Create() function
		HANDLE GetHandle() const
		{
			return m_hGrd;
		}

		// Get last error information from protected container with specified handle
		// The last error code is maintained on a per-handle basis
		// Multiple handles do not overwrite each other's last-error code
		int GetLastError(void** ppLastErrFunc = NULL) const
		{
			return GrdGetLastError(m_hGrd, ppLastErrFunc);
		}

		// Convert Guardant error code to text message string
		int FormatMessage(
				int   nErrorCode,     // Guardant error code
				int   nLanguage,      // Guardant language ID
				char* szErrorMsg,     // Pointer to a buffer for the formatted (and null-terminated) message.
				int   nErrorMsgSize,  // this parameter specifies the maximum number of characters that can be stored in the output buffer.
				void* pReserved = NULL) const // Reserved, must be NULL
		{
			return GrdFormatMessage(NULL, nErrorCode, nLanguage, szErrorMsg, nErrorMsgSize, pReserved);
		}

		// Convert Guardant error code to text message string
		int FormatMessage(
				int   nLanguage,      // Guardant language ID
				char* szErrorMsg,     // Pointer to a buffer for the formatted (and null-terminated) message.
				int   nErrorMsgSize,  // this parameter specifies the maximum number of characters that can be stored in the output buffer.
				void* pReserved = NULL) const // Reserved, must be NULL
		{
			return GrdFormatMessage(m_hGrd, 0, nLanguage, szErrorMsg, nErrorMsgSize, pReserved);
		}

		// Get requested information
		int GetInfo(
				DWORD dwInfoCode,       // Code of requested information. See GrdGIX_XXXXX
				void* pInfoData,        // Pointer to buffer for return data
				DWORD dwInfoSize) const // Number of bytes for returning
		{
			return GrdGetInfo(m_hGrd, dwInfoCode, pInfoData, dwInfoSize);
		}
		// template for get requested information of different types
		template <class Type>
		int GetInfo(
				DWORD dwInfoCode,     // Code of requested information. See GrdGIX_XXXXX
				Type* ptData) const   // Pointer to variable to be reterned
		{
			return GetInfo(dwInfoCode, ptData, sizeof(*ptData));
		}

		// Store dongle codes in Guardant protected container
		int SetAccessCodes(
				DWORD dwPublic,               // Must be already specified
				DWORD dwPrivateRD,            // Must be already specified
				DWORD dwPrivateWR     = 0,    // == 0 if not needed
				DWORD dwPrivateMST    = 0)    // == 0 if not needed
		{
			return GrdSetAccessCodes(m_hGrd, dwPublic, dwPrivateRD, dwPrivateWR, dwPrivateMST);
		}

		// Set dongle working mode to Guardant protected container
		int SetWorkMode(
				DWORD dwFlagsWork = GrdWM_UAM,            // combination of GrdWM_XXX flags
				DWORD dwFlagsMode = GrdWMFM_DriverAuto)   // combination of GrdWMFM_XXX flags
		{
			return GrdSetWorkMode(m_hGrd, dwFlagsWork, dwFlagsMode);
		}

		// Set dongle search conditions and operation modes to Guardant protected container
		int SetFindMode(
				DWORD dwRemoteMode = GrdFMR_Local + GrdFMR_Remote, // Local and/or remote (GrdFMR_Local + GrdFMR_Remote)
				DWORD dwFlags      = 0,                   // Combination of GrdFM_XXXXX flags
				DWORD dwProg       = 0,
				DWORD dwID         = 0,
				DWORD dwSN         = 0,
				DWORD dwVer        = 0,
				DWORD dwMask       = 0,
				DWORD dwType       = 0,
				DWORD dwModels     = GrdFMM_ALL,          // Possible dongle model bits. See GrdFMM_GSX definition
				DWORD dwInterfaces = GrdFMI_ALL)          // Possible dongle interface bits. See GrdFMI_XXX definition
		{
			return GrdSetFindMode(m_hGrd, dwRemoteMode, dwFlags, dwProg, dwID, dwSN, dwVer, dwMask, dwType, dwModels, dwInterfaces);
		}

		// Obtain the ID and other dongle info of the first or next dongle found
		int Find(
				DWORD  dwMode,                             // GrdF_First or GrdF_Next
				DWORD* pdwID,                              // Pointer to variable for return dongle ID
				TGrdFindInfo* pFindInfo = NULL) const      // Place for return dongle info or NULL for ignore it in other languages
		{
			return GrdFind(m_hGrd, dwMode, pdwID, pFindInfo);
		}

		// Login to Guardant dongle
		int Login(
				DWORD dwModuleLMS  = -1,                  // module number or -1 if License Management System functions are not used
				DWORD dwLoginFlags = GrdLM_PerStation)    // Login flags (GrdLLM_PerStation or GrdLLM_PerHandle)
		{
			return GrdLogin(m_hGrd, dwModuleLMS, dwLoginFlags);
		}

		// Log out from Guardant dongle
		int Logout(
				DWORD dwMode = 0)                     // == NULL if not need
		{
			return GrdLogout(m_hGrd, dwMode);
		}

		// Increment lock counter of specified dongle
		int Lock(
				DWORD dwTimeoutWaitForUnlock  = 10000,    // Max GrdAPI unlock waiting time. -1 == infinity. 0 == no waiting
				DWORD dwTimeoutAutoUnlock     = 10000,    // Max dongle locking time in ms.  -1 == infinity. 0 == 10000 ms (10 sec)
				DWORD dwMode                  = GrdLM_Nothing)  // in default works like critical section
		{
			return GrdLock(m_hGrd, dwTimeoutWaitForUnlock, dwTimeoutAutoUnlock, dwMode);
		}

		// Unlock specified dongle
		int Unlock()
		{
			return GrdUnlock(m_hGrd);
		}

		// Check for dongle availability
		int Check() const
		{
			return GrdCheck(m_hGrd);
		}

		// Check for dongle availability and decrement GP executions counter
		// Requires Private write code to be specified in SetAccessCodes
		int DecGP()
		{
			return GrdDecGP(m_hGrd);
		}

		// GrdDongle fSeek analog
		// Moves the dongle memory pointer (if any) associated with handle to a new location
		int Seek(DWORD dwSeek)
		{
			return GrdSeek(m_hGrd, dwSeek);
		}

		// Read a block of bytes from the dongle's memory
		int Read(
				DWORD dwAddr,                            // Starting address in dongle memory to be read
				DWORD dwLng,                             // Length of data to be read
				void* pData,                             // Buffer for data to be read
				void* pReserved = NULL) const            // Reserved, must be NULL
		{
			return GrdRead(m_hGrd, dwAddr, dwLng, pData, pReserved);
		}

		// Read a variable from the dongle's memory
		template <class Type>
		int Read(
				DWORD dwAddr,                            // Starting address in dongle memory to be read
				Type* ptData) const                      // Pointer to variable to be read
		{
			return Read(dwAddr, sizeof(*ptData), ptData);
		}

		// Read a block of bytes from the dongle's memory
		int Read(
				DWORD dwLng,                             // Length of data to be read
				void* pData,                             // Buffer for data to be read
				void* pReserved = NULL) const            // Reserved, must be NULL
		{
			return Read(GrdSeekCur, dwLng, pData, pReserved);
		}

		// Read a variable from the dongle's memory
		template <class Type>
		int Read(Type* ptData) const                      // Pointer to variable to be read
		{
			return Read(GrdSeekCur, sizeof(*ptData), ptData);
		}

		// Write a block of bytes into the dongle's memory
		// Requires Private write code to be specified in SetAccessCodes
		int Write(
				DWORD       dwAddr,                         // Starting address in dongle memory to be written
				DWORD       dwLng,                          // Length of data to be written
				const void* pData,                          // Buffer for data to be written
				void*       pReserved = NULL)               // Reserved, must be NULL
		{
			return GrdWrite(m_hGrd, dwAddr, dwLng, pData, pReserved);
		}

		// write a variable into the dongle's memory
		template <class Type>
		int Write(
				DWORD       dwAddr,                       // Starting address in dongle memory to be written
				const Type* ptData)                       // Pointer to variable to be written
		{
			return Write(dwAddr, sizeof(*ptData), ptData, NULL);
		}

		// Write a block of bytes into the dongle's memory
		// Requires Private write code to be specified in SetAccessCodes
		int Write(
				DWORD       dwLng,                        // Length of data to be written
				const void* pData,                        // Buffer for data to be written
				void*       pReserved = NULL)             // Reserved, must be NULL
		{
			return Write(GrdSeekCur, dwLng, pData, pReserved);
		}

		// write a variable into the dongle's memory
		template <class Type>
		int Write(const Type* ptData)                     // Pointer to variable to be written
		{
			return Write(GrdSeekCur, sizeof(*ptData), ptData, NULL);
		}

		// Initialize the dongle's memory
		// Requires Private master code to be specified in SetAccessCodes
		int Init()
		{
			return GrdInit(m_hGrd);
		}

		// Implement locks / Specify the number of hardware algorithms
		// and LMS table address
		// Requires Private master code to be specified in SetAccessCodes
		int Protect(
				DWORD dwWrProt,                           // address of first BYTES which can by write in SAM
				DWORD dwRdProt,                           // address of first BYTES which can by read  in SAM
				DWORD dwNumFunc,                          // Number of hardware-implemented algorithms in the dongle
				DWORD dwTableLMS = 0,                     // For Net II: SAM address of the first byte of LMS Table in 2-byte words; For Net III: number of protected item that contains LMS Table
				DWORD dwGlobalFlags = 0,                  // Reserved, must be 0
				void* pReserved = NULL)                   // Reserved, must be NULL
		{
			return GrdProtect(m_hGrd, dwWrProt, dwRdProt, dwNumFunc, dwTableLMS, dwGlobalFlags, pReserved);
		}

		//--- Cryptographic functions

		// Transform a block of bytes using dongle's hardware-implemented algorithm (including GSII64)
		int Transform(DWORD dwAlgoNum, DWORD dwLng, void* pData,
				DWORD dwMethod = 0, // GrdAM_ECB + GrdAM_Encrypt,// if Stealth I or Fidus it must be 0. Otherwise - combination of GrdAM_XXX flags
				void* pIV = NULL) const                        // if Stealth I or Fidus it must be NULL. Otherwise - 8-bytes initialization vector
		{
			return GrdTransform(m_hGrd, dwAlgoNum, dwLng, pData, dwMethod, pIV);
		}

		// Transform a block of bytes using dongle's hardware-implemented algorithm (including GSII64)
		int TransformEx(DWORD dwAlgoNum, DWORD dwLng, void* pData,
				DWORD dwMethod = 0, // GrdAM_ECB + GrdAM_Encrypt,// if Stealth I or Fidus it must be 0. Otherwise - combination of GrdAM_XXX flags
				DWORD dwIVLng = 0,
				void* pIV = NULL) const                       // if Stealth I or Fidus it must be NULL. Otherwise - 8-bytes initialization vector
		{
			return GrdTransformEx(m_hGrd, dwAlgoNum, dwLng, pData, dwMethod, dwIVLng, pIV, NULL);
		}

		// Encrypt/decrypt a block of bytes using encryption algorithm
		int Crypt(
				DWORD dwAlgo,                            // Number of hardware- or software-implemented algorithm
				DWORD dwDataLng,                         // Data length
				void* pData,                             // Data for Encryption/Decryption
				DWORD dwMethod      = 0,                 // Encrypt/Decrypt, First/Next/Last, block chaining modes (ECB/OFB/...)
				void* pIV           = NULL,              // Initialization Vector
				const void* pKeyBuf = NULL,              // Encryption/decryption secret key for software-implemented algorithm (NULL if not used)
				void* pContext      = NULL) const        // Context for multiple-buffer encryption. Must be corresponded GrdXXXXXX_CONTEXT_SIZE bytes size
		{
			return GrdCrypt(m_hGrd, dwAlgo, dwDataLng, pData, dwMethod, pIV, pKeyBuf, pContext);
		}

		// Encrypt/decrypt a block of bytes using encryption algorithm
		int CryptEx(
				DWORD       dwAlgo,                      // Number of hardware- or software-implemented algorithm
				DWORD       dwDataLng,                   // Data length
				void*       pData,                       // Data for Encryption/Decryption
				DWORD       dwMethod   = 0,              // Encrypt/Decrypt, First/Next/Last, block chaining modes (ECB/OFB/...)
				DWORD       dwIVLng    = 0,
				void*       pIV        = NULL,           // Initialization Vector
				const void* pKeyBuf    = NULL,           // Encryption/decryption secret key for software-implemented algorithm (NULL if not used)
				void*       pContext   = NULL) const     // Context for multiple-buffer encryption. Must be corresponded GrdXXXXXX_CONTEXT_SIZE bytes size
		{
			return GrdCryptEx(m_hGrd, dwAlgo, dwDataLng, pData, dwMethod, dwIVLng, pIV, pKeyBuf, pContext, NULL);
		}

		// Hash calculation of a block of bytes
		int Hash(
				DWORD       dwSoftHash,                        // Number of hardware- or software-implemented algorithm
				DWORD       dwDataLng,                         // Data length
				const void* pData,                             // Data for hash calculation
				DWORD       dwMethod,                          // GrdSC_First/GrdSC_Next/GrdSC_Last
				void*       pDigest,                           // Pointer to memory allocated for hash on GrdSC_Last step
				const void* pKeyBuf    = NULL,                 // Hash calculation secret key for software-implemented algorithm (NULL if not used)
				void*       pContext   = NULL)                 // Context for multiple buffer calculation. Must be corresponded GrdXXXXXX_CONTEXT_SIZE bytes size
		{
			return GrdHash(m_hGrd, dwSoftHash, dwDataLng, pData, dwMethod, pDigest, pKeyBuf, pContext);
		}

		// Calculate 32-bit CRC of a memory block
		DWORD CRC(const void* pData, DWORD dwLng, DWORD dwPrevCRC = Grd_StartCRC) const
		{
			return GrdCRC(pData, dwLng, dwPrevCRC);
		}

		//--- Protected Item functions

		// Activate dongle Algorithm or Protected Item
		int PI_Activate(
				DWORD dwItemNum,                          // Algorithm or Protected Item number to be activated
				DWORD dwActivatePsw    = 0)               // Optional password. If not used, must be 0
		{
			return GrdPI_Activate(m_hGrd, dwItemNum, dwActivatePsw);
		}

		// Deactivate dongle Algorithm or Protected Item
		int PI_Deactivate(
				DWORD dwItemNum,                          // Algorithm or Protected Item number to be deactivated
				DWORD dwDeactivatePsw = 0)                // Optional password. If not used, must be 0
		{
			return GrdPI_Deactivate(m_hGrd, dwItemNum, dwDeactivatePsw);
		}

		// Read data from dongle Protected Item
		int PI_Read(
				DWORD   dwItemNum,                          // Algorithm or Protected Item number to be read
				DWORD   dwAddr,                             // Offset in Algorithm or Protected Item data
				DWORD   dwLng,                              // Number of bytes for reading
				void*   pData,                              // Pointer to buffer for read data
				DWORD   dwReadPsw   = 0,                    // Optional password. If not used, must be 0
				void*   pReserved  = NULL) const            // Reserved, must be NULL
		{
			return GrdPI_Read(m_hGrd, dwItemNum, dwAddr, dwLng, pData, dwReadPsw, pReserved);
		}

		// Update data in dongle Protected Item
		int PI_Update(
				DWORD       dwItemNum,                          // Algorithm or Protected Item number to be updated
				DWORD       dwAddr,                             // Offset in Algorithm or Protected Item data
				DWORD       dwLng,                              // Number of bytes for updating
				const void* pData,                              // Pointer to buffer with data to be written
				DWORD       dwUpdatePsw = 0,                    // Optional password. If not used, must be 0
				DWORD       dwMethod    = GrdUM_MOV,            // Update method. See GrdUM_XXX definitions
				void*       pReserved   = NULL)                 // Reserved, must be NULL
		{
			return GrdPI_Update(m_hGrd, dwItemNum, dwAddr, dwLng, pData, dwUpdatePsw, dwMethod, pReserved);
		}

		// --- Guardant Trusted Remote Update API

		// Write secret GSII64 remote update key for Guardant Secured Remote Update to the dongle
		// Requires Private master code to be specified in SetAccessCodes
		int TRU_SetKey(const void* pGSII64_Key128)              // Unique Trusted Remote Update GSII64 128-bit secret key
		{
			return GrdTRU_SetKey(m_hGrd, pGSII64_Key128);
		}

		// Generate encrypted question and initialize remote update procedure
		int TRU_GenerateQuestion(
				void*  pQuestion,                         // Pointer to Question                  8 bytes (64 bit)
				DWORD* pdwID,                             // Pointer to ID                        4 bytes
				DWORD* pdwPublic,                         // Pointer to Dongle Public Code        4 bytes
				void*  pHash) const                       // Pointer to Hash of previous 16 bytes 8 bytes
		{
			return GrdTRU_GenerateQuestion(m_hGrd, pQuestion, pdwID, pdwPublic, pHash);
		}

		// Generate encrypted question and initialize remote update procedure
		int TRU_GenerateQuestionTime(
				void*  pQuestion,                         // Pointer to Question                  8 bytes (64 bit)
				DWORD* pdwID,                             // Pointer to ID                        4 bytes
				DWORD* pdwPublic,                         // Pointer to Dongle Public Code        4 bytes
				QWORD* pqwDongleTime,                     // Pointer to Dongle Time (encrypted)   8 bytes
				DWORD  dwDeadTimesSize,
				QWORD* pqwDeadTimes,
				DWORD* pdwDeadTimesNumbers,
				void*  pHash,                             // Pointer to Hash of previous data
				void*  pReserved = NULL) const            // Reserved, must be NULL
		{
			return GrdTRU_GenerateQuestionTime(m_hGrd, pQuestion, pdwID, pdwPublic, pqwDongleTime, dwDeadTimesSize, pqwDeadTimes, pdwDeadTimesNumbers, pHash, pReserved);
		}

		// Decrypt and validate question
		int TRU_DecryptQuestion(
				DWORD       dwAlgoNum_GSII64,             // Dongle GSII64 algorithm number with same key as in remote dongle
				DWORD       dwAlgoNum_HashS3,             // Dongle HASH algorithm number with same key as in remote dongle
				void*       pQuestion,                    // Pointer to Question                  8 bytes (64 bit)
				DWORD       dwID,                         // ID                                   4 bytes
				DWORD       dwPublic,                     // Public Code                          4 bytes
				const void* pHash) const                  // Pointer to Hash of previous 16 bytes 8 bytes
		{
			return GrdTRU_DecryptQuestion(m_hGrd, dwAlgoNum_GSII64, dwAlgoNum_HashS3, pQuestion, dwID, dwPublic, pHash);
		}

		// Decrypt and validate question
		int TRU_DecryptQuestionTime(                      // GSII64 algorithm with the same key as in remote dongle
				DWORD       dwAlgoNum_GSII64,             // Dongle GSII64 algorithm number with same key as in remote dongle
				DWORD       dwAlgoNum_HashS3,             // Dongle HASH64 algorithm number with same key as in remote dongle
				void*       pQuestion,                    // Pointer to Question                  8 bytes (64 bit)
				DWORD       dwID,                         // ID                                   4 bytes
				DWORD       dwPublic,                     // Public Code                          4 bytes
				QWORD*      pqwDongleTime,                // Pointer to Dongle Time (encrypted)   8 bytes
				QWORD*      pqwDeadTimes,
				DWORD       dwDeadTimesNumbers,
				const void* pHash) const                  // Pointer to Hash of previous 16 bytes 8 bytes
		{
			return GrdTRU_DecryptQuestionTime(m_hGrd, dwAlgoNum_GSII64, dwAlgoNum_HashS3, pQuestion, dwID, dwPublic, pqwDongleTime, pqwDeadTimes, dwDeadTimesNumbers, pHash);
		}

		// Set Init & Protect parameters for Trusted Remote Update
		// This function must be called after GrdTRU_DecryptQuestion and before GrdTRU_EncryptAnswer functions
		// only if Init & Protect operations will be executed during remote update (call GrdTRU_ApplyAnswer) procedure on remote PC
		int TRU_SetAnswerProperties(
				DWORD   dwTRU_Flags,        // Use Init & Protect or not
				DWORD   dwReserved,         // Reserved, must be 0
				DWORD   dwWrProt,           // remote GrdProtect parameters, SAM addres of the first byte available for writing in bytes
				DWORD   dwRdProt,           // remote GrdProtect parameters, SAM addres of the first byte available for reading in bytes
				DWORD   dwNumFunc,          // remote GrdProtect parameters, Number of hardware-implemented algorithms in the dongle including all protected items and LMS table of Net III
				DWORD   dwTableLMS  = 0,    // remote GrdProtect parameters, Net II: SAM address of the first byte of LMS Table in 2-byte words;
				// Net III: number of protected item that contains LMS Table
				DWORD   dwGlobalFlags = 0,  // remote GrdProtect parameters, Reserved, must be 0
				void*   pReserved  = NULL)  // remote GrdProtect parameters, Reserved, must be NULL
		{
			return GrdTRU_SetAnswerProperties(m_hGrd, dwTRU_Flags, dwReserved, dwWrProt, dwRdProt, dwNumFunc, dwTableLMS,
					dwGlobalFlags, pReserved);
		}

		// Prepare data for Trusted Remote Update
		int TRU_EncryptAnswer(
				DWORD       dwAddr,              // Starting address for writing in dongle
				DWORD       dwLng,               // Size of data to be written
				const void* pData,               // Buffer for data to be written
				const void* pQuestion,           // Pointer to decrypted Question            8 bytes (64 bit)
				DWORD       dwAlgoNum_GSII64,    // Dongle GSII64 algorithm number with the same key as in remote dongle
				DWORD       dwAlgoNum_HashS3,    // Dongle HASH algorithm number with the same key as in remote dongle
				void*       pAnswer,             // Pointer to the buffer for Answer data
				DWORD*      pdwAnswerSize) const // IN: Maximum buffer size for Answer data, OUT: Size of pAnswer buffer
		{
			return GrdTRU_EncryptAnswer(m_hGrd, dwAddr, dwLng, pData, pQuestion, dwAlgoNum_GSII64, dwAlgoNum_HashS3,
					pAnswer, pdwAnswerSize);
		}

		// Write data from secured buffer received via remote update procedure
		int TRU_ApplyAnswer(
				const void* pAnswer,        // Answer data update buffer prepared and encrypted by GrdTRU_EncryptAnswer
				DWORD       dwAnswerSize)   // Size of pAnswer buffer
		{
			return GrdTRU_ApplyAnswer(m_hGrd, pAnswer, dwAnswerSize);
		}

		// Digitally sign a block of bytes by using dongle hardware implemented ECC algorithm
		int Sign(
				DWORD       dwAlgoNum,              // Number of hardware implemented ECC algorithm
				DWORD       dwDataLng,              // Data for sign length (20 bytes for ECC160)
				const void* pData,                  // Data for sign
				DWORD       dwSignResultLng,        // ECC sign length (40 bytes for ECC160)
				void*       pSignResult,            // ECC sign
				void*       pReserved = NULL) const // Reserved, must be NULL
		{
			return GrdSign(m_hGrd, dwAlgoNum,  dwDataLng, pData, dwSignResultLng, pSignResult, pReserved);
		}

		// ECC algorithm digest verifying. Full software implemented
		int VerifySign(
				DWORD       dwAlgoType,              // Type of asymmetric cryptoalgorithm. See GrdVSC_XXXXX definition
				DWORD       dwPublicKeyLng,          // Public ECC key length
				const void* pPublicKey,              // Public ECC key
				DWORD       dwDataLng,               // Data for sign length (20 bytes for ECC160)
				const void* pData,                   // Data for sign
				DWORD       dwSignLng,               // ECC sign length (40 bytes for ECC160)
				const void* pSign,                   // ECC sign
				void*       pReserved = NULL) const  // Reserved, must be NULL
		{
			return GrdVerifySign(m_hGrd, dwAlgoType, dwPublicKeyLng, pPublicKey, dwDataLng, pData, dwSignLng, pSign, pReserved);
		}

		// Set dongle system time
		int SetTime(
				const TGrdSystemTime* pGrdSystemTime, // Pointer to TGrdSystemTime
				void*    pReserved)                   // Reserved, must be NULL
		{
			return GrdSetTime(m_hGrd, pGrdSystemTime, pReserved);
		}

		// Get dongle system time
		int GetTime(
				TGrdSystemTime* pGrdSystemTime, // Pointer to TGrdSystemTime
				void*    pReserved) const       // Reserved, must be NULL
		{
			return GrdGetTime(m_hGrd, pGrdSystemTime, pReserved);
		}

		// Get time limit for specified item
		int PI_GetTimeLimit(
				DWORD   dwItemNum,              // Algorithm or Protected Item number
				TGrdSystemTime* pGrdSystemTime, // Pointer to TGrdSystemTime
				void*    pReserved) const       // Reserved, must be NULL
		{
			return GrdPI_GetTimeLimit(m_hGrd, dwItemNum, pGrdSystemTime, pReserved);
		}

		// Get counter for specified item
		int PI_GetCounter(
				DWORD  dwItemNum,              // Algorithm or Protected Item number
				DWORD* pdwCounter,             // Pointer to counter value
				void*  pReserved) const        // Reserved, must be NULL
		{
			return GrdPI_GetCounter(m_hGrd, dwItemNum, pdwCounter, pReserved);
		}

		// Create a system time from components
		int MakeSystemTime(
				WORD    wYear,                        // The year (2000 - 2099)
				WORD    wMonth,                       // The month (January = 1, February = 2, ...)
				WORD    wDayOfWeek,                   // The day of the week (Sunday = 0, Monday = 1, ...)
				WORD    wDay,                         // The day of the month (1-31)
				WORD    wHour,                        // The hour (0-23)
				WORD    wMinute,                      // The minute (0-59)
				WORD    wSecond,                      // The second (0-59)
				WORD    wMilliseconds,                // The millisecond (0-999)
				TGrdSystemTime* pGrdSystemTime) const // Pointer to destination system time
		{
			return GrdMakeSystemTime(m_hGrd, wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds, pGrdSystemTime);
		}

		// Break a system time into components
		int SplitSystemTime(
				const TGrdSystemTime* pGrdSystemTime, // Pointer to source system time
				WORD*    pwYear,                    // Pointer for return year value
				WORD*    pwMonth,                   // Pointer for return month value
				WORD*    pwDayOfWeek,               // Pointer for return day of the week value
				WORD*    pwDay,                     // Pointer for return day of the month value
				WORD*    pwHour,                    // Pointer for return hour value
				WORD*    pwMinute,                  // Pointer for return minute value
				WORD*    pwSecond,                  // Pointer for return second value
				WORD*    pwMilliseconds) const      // Pointer for return millisecond value
		{
			return GrdSplitSystemTime(m_hGrd, pGrdSystemTime, pwYear, pwMonth, pwDayOfWeek, pwDay, pwHour, pwMinute, pwSecond, pwMilliseconds);
		}

		// Get information from user-defined loadable code descriptor
		int CodeGetInfo(
				DWORD dwAlgoName,                   // Algorithm numerical name to be loaded
				DWORD dwLng,                        // Number of bytes for reading (size of TGrdCodeInfo)
				void* pBuf,                         // Buffer for data to be read (pointer to TGrdCodeInfo)
				void* pReserved) const              // Reserved
		{
			return GrdCodeGetInfo(m_hGrd, dwAlgoName, dwLng, pBuf, pReserved);
		}

		// Load GCEXE file to the dongle
		int CodeLoad(
				DWORD       dwAlgoName,                  // Algorithm numerical name to be loaded
				DWORD       dwFileSize,                  // Buffer size for GCEXE-file
				const void* pFileBuf,                    // Pointer to the buffer for GCEXE-file
				void*       pReserved)                   // Reserved
		{
			return GrdCodeLoad(m_hGrd, dwAlgoName, dwFileSize, pFileBuf, pReserved);
		}

		// Run user-defined loadable code
		int CodeRun(
				DWORD       dwAlgoName,                  // Algorithm numerical name to be loaded
				DWORD       dwP1,                        // Parameter (subfunction code) for loadable code
				DWORD*      pdwRet,                      // Return code of loadable code
				DWORD       dwDataFromDongleLng,         // The amount of data to be received from the dongle
				void*       pDataFromDongle,             // Pointer to a buffer for data to be received from the dongle
				DWORD       dwDataToDongleLng,           // The amount of data to be sent to the dongle
				const void* pDataToDongle,               // Pointer to a buffer for data to be sent to the dongle
				void*       pReserved)                   // Reserved
		{
			return GrdCodeRun(m_hGrd, dwAlgoName, dwP1, pdwRet, dwDataFromDongleLng, pDataFromDongle,
					dwDataToDongleLng, pDataToDongle, pReserved);
		}

		// Switching driver type of USB-dongle
		int SetDriverMode(
				DWORD dwMode,                       // New Guardant dongle USB driver mode. See GrdDM_XXX definitions
				void* pReserved)                    // Reserved
		{
			return GrdSetDriverMode(m_hGrd, dwMode, pReserved);
		}

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64)
		// Register dongle notification for specified handle
		int RegisterDongleNotification(
				GrdDongleNotifyCallBack* pCallBack) const  // Pointer to Dongle Notification CallBack routine
		{
			// Initialize this copy of Guardant Dongle Notification API. GrdInitializeNotificationAPI() must be called once
			static InitializeDongleNotification InitDongleNotification;
			return GrdRegisterDongleNotification(m_hGrd, pCallBack);
		}

		// Unregister dongle notification for specified handle
		int UnRegisterDongleNotification() const
		{
			return GrdUnRegisterDongleNotification(m_hGrd);
		}
#endif // __WIN32__ || _WIN32 || WIN32 || _WIN64

};

#endif // !GRD_DONGLE_H
