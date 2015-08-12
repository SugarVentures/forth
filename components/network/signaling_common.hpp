#ifndef OPPVS_SIGNALING_COMMON_HPP
#define OPPVS_SIGNALING_COMMON_HPP

namespace oppvs {
	const static int DEFAULT_SIGNALING_PORT = 33333;
	const static int MAX_SIGNALING_MESSAGE_SIZE = 1000;
	const static int SIGNALING_HEADER_SIZE = 8;

	enum SignalingUserRole
	{
		SignalingBroadcaster = 0x01,
		SignalingViewer = 0x02
	};

	enum SignalingMessageType
	{
		SignalingStreamRegister = 0x01,
		SignalingIceRequest = 0x02,
		SignalingIceResponse = 0x03,
		SignalingStreamRequest = 0x04,
		SignalingStreamResponse = 0x05,
		SignalingInvalid = 0x00
	};

	const static uint16_t SIGNALING_ATTRIBUTE_ICE_USERNAME = 0x0001;
	const static uint16_t SIGNALING_ATTRIBUTE_ICE_PASSWORD = 0x0002;

	inline SignalingMessageType convertToSignalingMessageType(uint16_t value)
	{
		switch (value)
		{
			case 1:
				return SignalingStreamRegister;
			case 2:
				return SignalingIceRequest;
			case 3:
				return SignalingIceResponse;
			case 4:
				return SignalingStreamRequest;
			case 5:
				return SignalingStreamResponse;
			default:
				return SignalingInvalid;
		}
	}
} // oppvs

#endif // OPPVS_SIGNALING_COMMON_HPP
