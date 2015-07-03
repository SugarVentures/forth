/*
	STUN Headers

	RFC 5389: STUN RFC
	RFC 3489: OLD STUN RFC. Obsolete, but newer specs reference it
	RFC 5769 - test vectors for STUN
	RFC 5780 - Nat behavior discovery using STUN
*/

#ifndef OPPVS_STUN_HPP
#define OPPVS_STUN_HPP



namespace oppvs
{
	const uint16_t DEFAULT_STUN_PORT = 3478;
	const uint16_t DEFAULT_STUN_TLS_PORT = 5349;


	const uint16_t STUN_ATTRIBUTE_MAPPEDADDRESS   = 0x0001;
	const uint16_t STUN_ATTRIBUTE_RESPONSEADDRESS = 0x0002;
	const uint16_t STUN_ATTRIBUTE_CHANGEREQUEST   = 0x0003;
	const uint16_t STUN_ATTRIBUTE_SOURCEADDRESS   = 0x0004;
	const uint16_t STUN_ATTRIBUTE_CHANGEDADDRESS  = 0x0005; // this is the legacy "other address" from rfc 3489, superceded by STUN_ATTRIBUTE_OTHERADDRESS below

	const uint16_t STUN_ATTRIBUTE_USERNAME = 0x0006;

	const uint16_t STUN_ATTRIBUTE_LEGACY_PASSWORD = 0x0007; // old rfc


	const uint16_t STUN_ATTRIBUTE_MESSAGEINTEGRITY = 0x0008;
	const uint16_t STUN_ATTRIBUTE_ERRORCODE = 0x0009;
	const uint16_t STUN_ATTRIBUTE_UNKNOWNATTRIBUTES = 0x000A;

	const uint16_t STUN_ATTRIBUTE_REFLECTEDFROM = 0x000B; // old rfc


	const uint16_t STUN_ATTRIBUTE_REALM = 0x0014;
	const uint16_t STUN_ATTRIBUTE_NONCE = 0x0015;
	const uint16_t STUN_ATTRIBUTE_XORMAPPEDADDRESS = 0x0020;

	const uint16_t STUN_ATTRIBUTE_PADDING = 0x0026;
	const uint16_t STUN_ATTRIBUTE_RESPONSE_PORT = 0x0027;



	// This attribute is sent by the server to legacy clients
	// 0x8020 is is not defined in any RFC, but is the value that Vovida server uses
	const uint16_t STUN_ATTRIBUTE_XORMAPPEDADDRESS_OPTIONAL = 0x8020;

	const uint16_t STUN_ATTRIBUTE_SOFTWARE = 0x8022;
	const uint16_t STUN_ATTRIBUTE_ALTERNATESERVER = 0x8023;

	const uint16_t STUN_ATTRIBUTE_FINGERPRINT = 0x8028;

	const uint16_t STUN_ATTRIBUTE_RESPONSE_ORIGIN = 0x802b;
	const uint16_t STUN_ATTRIBUTE_OTHER_ADDRESS = 0x802c;




	const uint16_t STUN_TRANSACTION_ID_LENGTH = 16;

	const uint8_t STUN_ATTRIBUTE_FIELD_IPV4 = 1;
	const uint8_t STUN_ATTRIBUTE_FIELD_IPV6 = 2;

	const uint16_t STUN_IPV4_LENGTH = 4;
	const uint16_t STUN_IPV6_LENGTH = 16;


	const uint16_t STUN_ERROR_TRYALTERNATE = 300;
	const uint16_t STUN_ERROR_BADREQUEST = 400;
	const uint16_t STUN_ERROR_UNAUTHORIZED = 401;
	const uint16_t STUN_ERROR_UNKNOWNATTRIB = 420;
	const uint16_t STUN_ERROR_STALENONCE = 438;
	const uint16_t STUN_ERROR_SERVERERROR = 500;

	enum STUNMessageType
	{
		STUNBindingRequest,
		STUNResponse,

	};

	#define STUN_IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
	#define STUN_IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
	#define STUN_IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
	#define STUN_IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)
}

#endif