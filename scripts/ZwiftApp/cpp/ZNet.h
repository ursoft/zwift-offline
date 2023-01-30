#pragma once

namespace zwift_network {
	enum NetworkRequestOutcome { NRO_NULL, NRO_CNT };
}
namespace ZNet {
	struct Error {
		Error(std::string_view msg, zwift_network::NetworkRequestOutcome netReqOutcome) : m_msg(msg), m_netReqOutcome(netReqOutcome), m_hasNetReqOutcome(true) {}
		Error(std::string_view msg) : m_msg(msg) {}

		std::string_view m_msg;
		zwift_network::NetworkRequestOutcome m_netReqOutcome = zwift_network::NRO_NULL;
		bool m_hasNetReqOutcome = false;
	};
}