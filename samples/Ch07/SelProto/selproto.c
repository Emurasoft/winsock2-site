//
// SELPROTO.C  -- SelectProtocols()�֐�
//

#include <winsock2.h>

////////////////////////////////////////////////////////////
// int SelectProtocols(
//					   DWORD dwSetFlags,
//					   DWORD dwNotSetFlags,
//					   LPWSAPROTOCOL_INFO lpProtocolBuffer,
//					   LPDWORD lpdwBufferLength
//					   )
//
// Retrieves information about available transport protocols
// that meet the criteria indicated by dwSetFlags and 
// dwNotSetFlags
//
// PARAMETERS
//
// dwSetFlags			A bitmask of values to ensure
//						are set in the protocol's
//						WSAPROTOCOL_INFO.dwServiceFlags1 
//						field.
//
// dwNotSetFlags		A bitmask of values to ensure
//						are NOT set in the protocol's
//						WSAPROTOCOL_INFO.dwServiceFlags1 
//						field.
//
// lpProtocolBuffer		The buffer to be filled with
//						WSAPROTOCOL_INFO structures.
//
// lpdwBufferLength		On input, the size in bytes of 
//						the buffer pointed to by the
//						lpProtocolBuffer parameter.
//						On ouput, the minimum size the
//						buffer must be to retrieve all
//						of the requested information.
//
// REMARKS
//
// SelectProtocols() fills the supplied buffer with 
// WSAPROTOCOL_INFO structures for all protocols where the 
// flags specified in dwSetFlags are set and all flags in 
// dwNotSetFlags are NOT set.
//
// Example: To retrieve information about all installed 
// protocols that are connection-oriented and reliable,
// call SelectProtocols() like this:
//
// nRet = SelectProtocols(XP1_GUARANTEED_DELIVERY|XP1_GUARANTEED_ORDER,
//						  XP1_CONNECTIONLESS,
//						  lpProtocolBuffer,
//						  &dwBufLen);
//
// In this example, SelectProtocols() will return information
// about all installed protocols and protocol chains that
// have the XP1_GUARANTEED_DELIVERY and XP1_GUARANTEED_ORDER
// flags set and that also don't have XP1_CONNECTIONLESS set.
//
// RETURN VALUES
//
// If no error occurs, SelectProtocols() returns the number of
// WSAPROTOCOL_INFO structure copied to the supplied buffer.
// If an error occurs, it returns SOCKET_ERROR and a specific
// error code can be found with WSAGetLastError();
//
int SelectProtocols(
					DWORD dwSetFlags,
					DWORD dwNotSetFlags,
					LPWSAPROTOCOL_INFO lpProtocolBuffer,
					LPDWORD lpdwBufferLength
					)
{
	LPBYTE				pBuf;
	LPWSAPROTOCOL_INFO	pInfo;
	DWORD				dwNeededLen;
	LPWSAPROTOCOL_INFO	pRetInfo;
	DWORD				dwRetLen;
	int					nCount;
	int					nMatchCount;
	int					nRet;

	//
	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂ�
	dwNeededLen = 0;
	nRet = WSAEnumProtocols(NULL, NULL, &dwNeededLen);
	if (nRet == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAENOBUFS)
			return SOCKET_ERROR;
	}

	//
	// �o�b�t�@�����蓖�Ă�
	//
	pBuf = malloc(dwNeededLen);
	if (pBuf == NULL)
	{
		WSASetLastError(WSAENOBUFS);
		return SOCKET_ERROR;
	}

	//
	// ���ۂ̌Ăяo�����s��
	//
	nRet = WSAEnumProtocols(NULL, 
							(LPWSAPROTOCOL_INFO)pBuf, 
							&dwNeededLen);
	if (nRet == SOCKET_ERROR)
	{
		free(pBuf);
		return SOCKET_ERROR;
	}

	//
	// �v���g�R����I�����邽�߂̃w���p�[�}�N��
	//
	#define REJECTSET(f) \
	    ((dwSetFlags & f) && !(pInfo->dwServiceFlags1 & f))
	#define REJECTNOTSET(f) \
	    ((dwNotSetFlags &f) && (pInfo->dwServiceFlags1 & f))
	#define REJECTEDBY(f) (REJECTSET(f) || REJECTNOTSET(f))

	//
	// ���[�v�����Ŋe�v���g�R����I������
	//
	pInfo = (LPWSAPROTOCOL_INFO)pBuf;	
	pRetInfo = lpProtocolBuffer;
	dwRetLen = 0;
	nMatchCount = 0;
	for(nCount = 0; nCount < nRet; nCount++)
	{
		//
		// �v�����ꂽ���ׂẴt���O���`�F�b�N����
		//
		while(1)
		{
			if (REJECTEDBY(XP1_CONNECTIONLESS))
				break;
			if (REJECTEDBY(XP1_GUARANTEED_DELIVERY))
				break;
			if (REJECTEDBY(XP1_GUARANTEED_ORDER))
				break;
			if (REJECTEDBY(XP1_MESSAGE_ORIENTED))
				break;
			if (REJECTEDBY(XP1_PSEUDO_STREAM))
				break;
			if (REJECTEDBY(XP1_GRACEFUL_CLOSE))
				break;
			if (REJECTEDBY(XP1_EXPEDITED_DATA))
				break;
			if (REJECTEDBY(XP1_CONNECT_DATA))
				break;
			if (REJECTEDBY(XP1_DISCONNECT_DATA))
				break;
			if (REJECTEDBY(XP1_SUPPORT_BROADCAST)) 
				break;
			if (REJECTEDBY(XP1_SUPPORT_MULTIPOINT))
				break;
			if (REJECTEDBY(XP1_MULTIPOINT_DATA_PLANE))
				break;
			if (REJECTEDBY(XP1_QOS_SUPPORTED))
				break;
			if (REJECTEDBY(XP1_UNI_SEND))
				break;
			if (REJECTEDBY(XP1_UNI_RECV))
				break;
			if (REJECTEDBY(XP1_IFS_HANDLES))
				break;
			if (REJECTEDBY(XP1_PARTIAL_MESSAGE))
				break;
			//
			// �����܂ŗ����ꍇ�́A
			// �v���g�R���͂��ׂĂ̗v���𖞂����Ă���
			//
			dwRetLen += sizeof(WSAPROTOCOL_INFO);
			if (dwRetLen > *lpdwBufferLength)
			{
				// �w��̃o�b�t�@������������
				WSASetLastError(WSAENOBUFS);
				*lpdwBufferLength = dwNeededLen;
				free(pBuf);
				return SOCKET_ERROR;
			}
			nMatchCount++;
			// ���̃v���g�R�����Ăяo�����̃o�b�t�@�ɃR�s�[����
			memcpy(pRetInfo, pInfo, sizeof(WSAPROTOCOL_INFO));
			pRetInfo++;
			break;
		}
		pInfo++;
	}
	free(pBuf);
	*lpdwBufferLength = dwRetLen;
	return(nMatchCount);
}

////////////////////////////////////////////////////////////
