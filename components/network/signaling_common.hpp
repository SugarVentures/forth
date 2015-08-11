#ifndef OPPVS_SIGNALING_COMMON_HPP
#define OPPVS_SIGNALING_COMMON_HPP

namespace oppvs {
	const static int DEFAULT_SIGNALING_PORT = 33333;
	const static int MAX_SIGNALING_MESSAGE_SIZE = 1000;

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
		SignalingStreamResponse = 0x05
	};


} // oppvs

#endif // OPPVS_SIGNALING_COMMON_HPP
