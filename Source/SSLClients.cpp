/*
*
	Copyright 2021, 2022 Chris M. (RealTimeChris)

	This file is part of DiscordCoreAPI.
	DiscordCoreAPI is free software: you can redistribute it and/or modify it under the terms of the GNU
	General Public License as published by the Free Software Foundation, either version 3 of the License,
	or (at your option) any later version.
	DiscordCoreAPI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with DiscordCoreAPI.
	If not, see <https://www.gnu.org/licenses/>.

*/
/// SSLClents.cpp - Source file for the "SSL Client" stuff.
/// Dec 12, 2021
/// https://discordcoreapi.com
/// \file SSLClients.cpp

#include <discordcoreapi/SSLClients.hpp>

namespace DiscordCoreInternal {

	void reportSSLError(const std::string& errorPosition, int32_t errorValue = 0, SSL* ssl = nullptr) noexcept {
		if (ssl) {
			std::cout << DiscordCoreAPI::shiftToBrightRed() << errorPosition << SSL_get_error(ssl, errorValue) << std::endl;
		} else {
			std::cout << DiscordCoreAPI::shiftToBrightRed() << errorPosition << std::endl;
		}
		ERR_print_errors_fp(stdout);
		std::cout << std::endl << DiscordCoreAPI::reset();
	}

	void reportError(const std::string& errorPosition, int32_t errorValue) noexcept {
		std::cout << DiscordCoreAPI::shiftToBrightRed() << errorPosition << errorValue << ", ";
#ifdef _WIN32
		std::unique_ptr<char[]> string{ std::make_unique<char[]>(1024) };
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), string.get(), 1024, NULL);
		std::cout << WSAGetLastError() << ", " << string << std::endl << DiscordCoreAPI::reset();
#else
		std::cout << strerror(errno) << std::endl << DiscordCoreAPI::reset();
#endif
	}

	bool HttpSSLClient::connect(const std::string& baseUrl, bool doWePrintErrorNew, const std::string& portNew) noexcept {
		this->doWePrintErrorMessages = doWePrintErrorNew;
		std::string stringNew{};
		if (baseUrl.find(".com") != std::string::npos) {
			stringNew =
				baseUrl.substr(baseUrl.find("https://") + std::string("https://").size(), baseUrl.find(".com") + std::string(".com").size() - std::string("https://").size());
		} else if (baseUrl.find(".org") != std::string::npos) {
			stringNew =
				baseUrl.substr(baseUrl.find("https://") + std::string("https://").size(), baseUrl.find(".org") + std::string(".org").size() - std::string("https://").size());
		}

		addrinfoWrapper hints{ nullptr }, address{ nullptr };
		hints->ai_family = AF_INET;
		hints->ai_socktype = SOCK_STREAM;
		hints->ai_protocol = IPPROTO_TCP;

		if (auto returnValue = getaddrinfo(stringNew.c_str(), portNew.c_str(), hints, address); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("getaddrinfo() Error: ", returnValue);
			}
			return false;
		}

		if (this->theSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol); this->theSocket == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("socket() Error: ", this->theSocket);
			}
			return false;
		}

		int32_t value{ this->maxBufferSize + 1 };
		if (auto returnValue = setsockopt(this->theSocket, SOL_SOCKET, SO_SNDBUF, static_cast<char*>(static_cast<void*>(&value)), sizeof(value)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("setsockopt() Error: ", returnValue);
			}
			return false;
		}

#ifdef _WIN32
		char optionValue{ true };
		if (auto returnValue = setsockopt(this->theSocket, IPPROTO_TCP, TCP_NODELAY, &optionValue, sizeof(optionValue)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("setsockopt() Error: ", returnValue);
			}
			return false;
		}
#else
		int32_t optionValue{ 1 };
		if (auto returnValue = setsockopt(this->theSocket, SOL_TCP, TCP_NODELAY, &optionValue, sizeof(optionValue)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("setsockopt() Error: ", returnValue);
			}
			return false;
		}
#endif

		if (auto returnValue = ::connect(this->theSocket, address->ai_addr, static_cast<int32_t>(address->ai_addrlen)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("connect() Error: ", returnValue);
			}
			return false;
		}

		if (this->context = SSL_CTX_new(TLS_client_method()); this->context == nullptr) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_CTX_new() Error: ");
			}
			return false;
		}

		auto options{ SSL_CTX_get_options(this->context) };
		if (SSL_CTX_set_options(this->context, SSL_OP_IGNORE_UNEXPECTED_EOF) != (options | SSL_OP_IGNORE_UNEXPECTED_EOF)) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_CTX_set_options() Error: ");
			}
			return false;
		}

		if (this->ssl = SSL_new(this->context); this->ssl == nullptr) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_new() Error: ");
			}
			return false;
		}

		if (auto returnValue = SSL_set_fd(this->ssl, this->theSocket); !returnValue) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_set_fd() Error: ", returnValue, this->ssl);
			}
			return false;
		}

		if (auto returnValue = SSL_set_tlsext_host_name(this->ssl, stringNew.c_str()); !returnValue) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_set_tlsext_host_name() Error: ", returnValue, this->ssl);
			}
			return false;
		}

		if (auto returnValue = SSL_connect(this->ssl); !returnValue) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_connect() Error: ", returnValue, this->ssl);
			}
			return false;
		}

		return true;
	}

	void HttpSSLClient::writeData(const std::string& data) noexcept {
		this->outputBuffer.insert(this->outputBuffer.end(), data.begin(), data.end());
	}

	std::string& HttpSSLClient::getInputBuffer() noexcept {
		return this->inputBuffer;
	}

	bool HttpSSLClient::processIO() noexcept {
#ifdef _WIN32
		fd_set writeSet{}, readSet{};
		int32_t nfds{ 0 };
		FD_ZERO(&writeSet);
		FD_ZERO(&readSet);

		if (this->outputBuffer.size() > 0 && !this->wantRead) {
			FD_SET(this->theSocket, &writeSet);
			nfds = this->theSocket > nfds ? this->theSocket : nfds;
		}
		FD_SET(this->theSocket, &readSet);
		nfds = this->theSocket > nfds ? this->theSocket : nfds;

		timeval checkTime{ .tv_usec = 1000 };
		if (auto resultValue = select(nfds + 1, &readSet, &writeSet, nullptr, &checkTime); resultValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("select() Error: ", resultValue);
			}
			return false;
		} else if (resultValue == 0) {
			return true;
		}
#else
		epoll_event writeEvent{}, readEvent{}, events[1]{};
		epollWrapper epoll{ nullptr };
		bool writing{ false };

		if (this->outputBuffer.size() > 0 && !this->wantRead) {
			writing = true;
			writeEvent.events = EPOLLOUT;
			writeEvent.data.fd = this->theSocket;
			if (auto resultValue = epoll_ctl(epoll, EPOLL_CTL_ADD, this->theSocket, &writeEvent); resultValue == SOCKET_ERROR) {
				if (this->doWePrintErrorMessages) {
					reportError("epoll_ctl() Error: ", resultValue);
				}
				return false;
			}
		} else {
			readEvent.events = EPOLLIN;
			readEvent.data.fd = this->theSocket;
			if (auto resultValue = epoll_ctl(epoll, EPOLL_CTL_ADD, this->theSocket, &readEvent); resultValue == SOCKET_ERROR) {
				if (this->doWePrintErrorMessages) {
					reportError("epoll_ctl() Error: ", resultValue);
				}
				return false;
			}
		}

		if (auto resultValue = epoll_wait(epoll, events, 1, 1); resultValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("epoll_wait() Error: ", resultValue);
			}
			return false;
		} else if (resultValue == 0) {
			return true;
		}

#endif

#ifdef _WIN32
		if (FD_ISSET(this->theSocket, &readSet)) {
#else
		if (!writing) {
#endif
			this->wantRead = false;
			std::string serverToClientBuffer{};
			serverToClientBuffer.resize(this->maxBufferSize);
			size_t readBytes{ 0 };
			auto returnValue{ SSL_read_ex(this->ssl, serverToClientBuffer.data(), this->maxBufferSize, &readBytes) };
			auto errorValue{ SSL_get_error(this->ssl, returnValue) };
			switch (errorValue) {
				case SSL_ERROR_NONE: {
					if (readBytes > 0) {
						this->inputBuffer.insert(this->inputBuffer.end(), serverToClientBuffer.begin(), serverToClientBuffer.begin() + readBytes);
					}
					return true;
				}
				case SSL_ERROR_SYSCALL: {
					[[fallthrough]];
				}
				case SSL_ERROR_ZERO_RETURN: {
					if (this->doWePrintErrorMessages) {
						reportSSLError("HttpSSLClient::processIO::SSL_read_ex() Error: ", returnValue, this->ssl);
						reportError("HttpSSLClient::processIO::SSL_read_ex() Error: ", returnValue);
					}
					return false;
				}
				case SSL_ERROR_WANT_READ: {
					this->wantRead = true;
					[[fallthrough]];
				}
				case SSL_ERROR_WANT_WRITE: {
					return true;
				}
				default: {
					if (this->doWePrintErrorMessages) {
						reportSSLError("HttpSSLClient::processIO::SSL_read_ex() Error: ", returnValue, this->ssl);
						reportError("HttpSSLClient::processIO::SSL_read_ex() Error: ", returnValue);
					}
					return false;
				}
			}
		}
#ifdef _WIN32
		else if (FD_ISSET(this->theSocket, &writeSet)) {
#else
		else {
#endif
			size_t writtenBytes{ 0 };
			auto returnValue{ SSL_write_ex(this->ssl, this->outputBuffer.data(), this->outputBuffer.size(), &writtenBytes) };
			auto errorValue{ SSL_get_error(this->ssl, returnValue) };
			switch (errorValue) {
				case SSL_ERROR_NONE: {
					this->outputBuffer.clear();
					return true;
				}
				case SSL_ERROR_SYSCALL: {
					[[fallthrough]];
				}
				case SSL_ERROR_ZERO_RETURN: {
					if (this->doWePrintErrorMessages) {
						reportSSLError("HttpSSLClient::processIO::SSL_write_ex() Error: ", returnValue, this->ssl);
						reportError("HttpSSLClient::processIO::SSL_write_ex() Error: ", returnValue);
					}
					return false;
				}
				case SSL_ERROR_WANT_READ: {
					this->wantRead = true;
					[[fallthrough]];
				}
				case SSL_ERROR_WANT_WRITE: {
					return true;
				}
				default: {
					if (this->doWePrintErrorMessages) {
						reportSSLError("HttpSSLClient::processIO::SSL_write_ex() Error: ", returnValue, this->ssl);
						reportError("HttpSSLClient::processIO::SSL_write_ex() Error: ", returnValue);
					}
					return false;
				}
			}
		}
		return true;
	}

	WebSocketSSLClient::WebSocketSSLClient(const std::string& baseUrlNew, const std::string& portNew, bool doWePrintErrorNew,  int32_t maxBufferSizeNew) noexcept {
		this->doWePrintErrorMessages = doWePrintErrorNew;
		this->maxBufferSize = maxBufferSizeNew;
		addrinfoWrapper hints{ nullptr }, address{ nullptr };
		hints->ai_family = AF_INET;
		hints->ai_socktype = SOCK_STREAM;
		hints->ai_protocol = IPPROTO_TCP;

		if (auto returnValue = getaddrinfo(baseUrlNew.c_str(), portNew.c_str(), hints, address); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("getaddrinfo() Error: ", returnValue);
			}
			return;
		}

		if (this->theSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol); this->theSocket == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("socket() Error: ", this->theSocket);
			}
			return;
		}

		int32_t value{ this->maxBufferSize + 1 };
		if (auto returnValue = setsockopt(this->theSocket, SOL_SOCKET, SO_SNDBUF, static_cast<char*>(static_cast<void*>(& value)), sizeof(value)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("setsockopt() Error: ", returnValue);
			}
			return;
		}

#ifdef _WIN32
		char optionValue{ true };
		if (auto returnValue = setsockopt(this->theSocket, IPPROTO_TCP, TCP_NODELAY, &optionValue, sizeof(optionValue)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("setsockopt() Error: ", returnValue);
			}
			return;
		}
#else
		int32_t optionValue{ 1 };
		if (auto returnValue = setsockopt(this->theSocket, SOL_TCP, TCP_NODELAY, &optionValue, sizeof(optionValue)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("setsockopt() Error: ", returnValue);
			}
			return;
		}
#endif

		if (auto returnValue = connect(this->theSocket, address->ai_addr, static_cast<int32_t>(address->ai_addrlen)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("connect() Error: ", returnValue);
			}
			return;
		}

		if (this->context = SSL_CTX_new(TLS_client_method()); this->context == nullptr) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_CTX_new() Error: ");
			}
			return;
		}

		auto options{ SSL_CTX_get_options(this->context) };
		if (SSL_CTX_set_options(this->context, SSL_OP_IGNORE_UNEXPECTED_EOF) != (options | SSL_OP_IGNORE_UNEXPECTED_EOF)) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_CTX_set_options() Error: ");
			}
			return;
		}

		if (this->ssl = SSL_new(this->context); this->ssl == nullptr) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_new() Error: ");
			}
			return;
		}

		if (auto returnValue = SSL_set_fd(this->ssl, this->theSocket); !returnValue) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_set_fd() Error: ", returnValue, this->ssl);
			}
			return;
		}

		/* SNI */
		if (auto returnValue = SSL_set_tlsext_host_name(this->ssl, baseUrlNew.c_str()); !returnValue) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_set_tlsext_host_name() Error: ", returnValue, this->ssl);
			}
			return;
		}

		if (auto returnValue = SSL_connect(this->ssl); !returnValue) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("SSL_connect() Error: ", returnValue, this->ssl);
			}
			return;
		}

		this->areWeConnected = true;
	};

	bool WebSocketSSLClient::processIO(int32_t waitTimeInMicroSeconds) noexcept {
		if (this->areWeConnected) {
#ifdef _WIN32
			fd_set writeSet{}, readSet{};
			int32_t nfds{ 0 };
			FD_ZERO(&writeSet);
			FD_ZERO(&readSet);

			if (this->outputBuffer.size() > 0 && !this->wantRead) {
				FD_SET(this->theSocket, &writeSet);
				nfds = this->theSocket > nfds ? this->theSocket : nfds;
			}
			FD_SET(this->theSocket, &readSet);
			nfds = this->theSocket > nfds ? this->theSocket : nfds;

			timeval checkTime{ .tv_usec = waitTimeInMicroSeconds };
			if (auto resultValue = select(nfds + 1, &readSet, &writeSet, nullptr, &checkTime); resultValue == SOCKET_ERROR) {
				if (this->doWePrintErrorMessages) {
					reportError("select() Error: ", resultValue);
				}
				this->areWeConnected = false;
				return false;
			} else if (resultValue == 0) {
				return true;
			}
#else
			epoll_event writeEvent{}, readEvent{}, events[1]{};
			epollWrapper epoll{ nullptr };
			bool writing{ false };

			if (this->outputBuffer.size() > 0 && !this->wantRead) {
				writing = true;
				writeEvent.events = EPOLLOUT;
				writeEvent.data.fd = this->theSocket;
				if (auto resultValue = epoll_ctl(epoll, EPOLL_CTL_ADD, this->theSocket, &writeEvent); resultValue == SOCKET_ERROR) {
					if (this->doWePrintErrorMessages) {
						reportError("epoll_ctl() Error: ", resultValue);
					}
					this->areWeConnected = false;
					return false;
				}
			} else {
				readEvent.events = EPOLLIN;
				readEvent.data.fd = this->theSocket;
				if (auto resultValue = epoll_ctl(epoll, EPOLL_CTL_ADD, this->theSocket, &readEvent); resultValue == SOCKET_ERROR) {
					if (this->doWePrintErrorMessages) {
						reportError("epoll_ctl() Error: ", resultValue);
					}
					this->areWeConnected = false;
					return false;
				}
			}

			if (auto resultValue = epoll_wait(epoll, events, 1, waitTimeInMicroSeconds / 1000); resultValue == SOCKET_ERROR) {
				if (this->doWePrintErrorMessages) {
					reportError("epoll_wait() Error: ", resultValue);
				}
				this->areWeConnected = false;
				return false;
			} else if (resultValue == 0) {
				return true;
			}

#endif

#ifdef _WIN32
			if (FD_ISSET(this->theSocket, &readSet)) {
#else
			if (!writing) {
#endif
				this->wantRead = false;
				std::string serverToClientBuffer{};
				serverToClientBuffer.resize(this->maxBufferSize);
				size_t readBytes{ 0 };
				auto returnValue{ SSL_read_ex(this->ssl, serverToClientBuffer.data(), this->maxBufferSize, &readBytes) };
				auto errorValue{ SSL_get_error(this->ssl, returnValue) };
				switch (errorValue) {
					case SSL_ERROR_NONE: {
						if (readBytes > 0) {
							this->inputBuffer.insert(this->inputBuffer.end(), serverToClientBuffer.begin(), serverToClientBuffer.begin() + readBytes);
						}
						return true;
					}
					case SSL_ERROR_SYSCALL: {
						[[fallthrough]];
					}
					case SSL_ERROR_ZERO_RETURN: {
						if (this->doWePrintErrorMessages) {
							reportSSLError("WebSocketSSLClient::processIO::SSL_read_ex() Error: ", returnValue, this->ssl);
							reportError("WebSocketSSLClient::processIO::SSL_read_ex() Error: ", returnValue);
						}
						this->areWeConnected = false;
						return false;
					}
					case SSL_ERROR_WANT_READ: {
						this->wantRead = true;
						[[fallthrough]];
					}
					case SSL_ERROR_WANT_WRITE: {
						return true;
					}
					default: {
						if (this->doWePrintErrorMessages) {
							reportSSLError("WebSocketSSLClient::processIO::SSL_read_ex() Error: ", returnValue, this->ssl);
							reportError("WebSocketSSLClient::processIO::SSL_read_ex() Error: ", returnValue);
						}
						this->areWeConnected = false;
						return false;
					}
				}
			}
#ifdef _WIN32
			else if (FD_ISSET(this->theSocket, &writeSet)) {
#else
			else {
#endif
				this->wantRead = false;
				size_t writtenBytes{ 0 };
				auto returnValue{ SSL_write_ex(this->ssl, this->outputBuffer.data(), this->outputBuffer.size(), &writtenBytes) };
				auto errorValue{ SSL_get_error(this->ssl, returnValue) };
				switch (errorValue) {
					case SSL_ERROR_NONE: {
						this->outputBuffer.clear();
						return true;
					}
					case SSL_ERROR_SYSCALL: {
						[[fallthrough]];
					}
					case SSL_ERROR_ZERO_RETURN: {
						if (this->doWePrintErrorMessages) {
							reportSSLError("WebSocketSSLClient::processIO::SSL_write_ex() Error: ", returnValue, this->ssl);
							reportError("WebSocketSSLClient::processIO::SSL_write_ex() Error: ", returnValue);
						}
						this->areWeConnected = false;
						return false;
					}
					case SSL_ERROR_WANT_READ: {
						this->wantRead = true;
						[[fallthrough]];
					}
					case SSL_ERROR_WANT_WRITE: {
						return true;
					}
					default: {
						if (this->doWePrintErrorMessages) {
							reportSSLError("WebSocketSSLClient::processIO::SSL_write_ex() Error: ", returnValue, this->ssl);
							reportError("WebSocketSSLClient::processIO::SSL_write_ex() Error: ", returnValue);
						}
						this->areWeConnected = false;
						return false;
					}
				}
			}
			return true;
		} else {
			return false;
		}
	}

	void WebSocketSSLClient::writeData(const std::string& data) noexcept {
		this->outputBuffer.insert(this->outputBuffer.end(), data.begin(), data.end());
	}

	std::string WebSocketSSLClient::getInputBuffer() noexcept {
		std::string returnString = this->inputBuffer;
		this->inputBuffer.clear();
		return returnString;
	}

	int64_t WebSocketSSLClient::getBytesRead() noexcept {
		return this->bytesRead;
	}

	bool WebSocketSSLClient::didWeConnect() noexcept{
		return this->areWeConnected;
	}

	DatagramSocketSSLClient::DatagramSocketSSLClient(const std::string& baseUrlNew, const std::string& portNew, bool doWePrintErrorNew) noexcept {
		this->doWePrintErrorMessages = doWePrintErrorNew;
		addrinfoWrapper hints{ nullptr }, address{ nullptr };
		hints->ai_family = AF_INET;
		hints->ai_socktype = SOCK_DGRAM;
		hints->ai_protocol = IPPROTO_UDP;

		if (auto returnValue = getaddrinfo(baseUrlNew.c_str(), portNew.c_str(), hints, address); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("getaddrinfo() Error: ", returnValue);
			}
			return;
		}

		if (this->theSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol); this->theSocket == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("socket() Error: ", 0);
			}
			return;
		}

		if (auto returnValue = connect(this->theSocket, address->ai_addr, static_cast<int32_t>(address->ai_addrlen)); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("connect() Error: ", returnValue);
			}
			return;
		}

#ifdef _WIN32
		u_long value{ 1 };
		if (auto returnValue = ioctlsocket(this->theSocket, FIONBIO, &value); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("ioctlsocket() Error: ", returnValue);
			}
			return;
		}
#else
		if (auto returnValue = fcntl(this->theSocket, F_SETFL, fcntl(this->theSocket, F_GETFL, 0) | O_NONBLOCK); returnValue == SOCKET_ERROR) {
			if (this->doWePrintErrorMessages) {
				reportError("fcntl() Error: ", returnValue);
			}
			return;
		}
#endif
		if (this->datagramBio = BIO_new_dgram(this->theSocket, BIO_CLOSE); this->datagramBio == nullptr) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("BIO_new_dgram() Error: ");
			}
			return;
		}

		if (auto returnValue = BIO_ctrl(this->datagramBio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &address); returnValue == 0) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("BIO_ctrl() Error: ");
			}
			return;
		}

		return;
	}

	bool DatagramSocketSSLClient::writeData(const std::string& data) noexcept {
		size_t writtenBytes{ 0 };
		if (!BIO_write_ex(this->datagramBio, data.data(), data.size(), &writtenBytes)) {
			if (this->doWePrintErrorMessages) {
				reportSSLError("BIO_write_ex() Error: ");
			}
			return false;
		}
		return true;
	}

	std::string& DatagramSocketSSLClient::getInputBuffer() noexcept {
		return this->inputBuffer;
	}

	bool DatagramSocketSSLClient::readData(bool doWeClear) noexcept {
		std::string serverToClientBuffer{};
		serverToClientBuffer.resize(this->maxBufferSize);
		size_t readBytes{ 0 };
		if (auto returnValue = BIO_read_ex(this->datagramBio, serverToClientBuffer.data(), this->maxBufferSize, &readBytes); returnValue == 1) {
			if (readBytes > 0) {
				this->inputBuffer.insert(this->inputBuffer.end(), serverToClientBuffer.begin(), serverToClientBuffer.begin() + readBytes);
				if (doWeClear) {
					this->inputBuffer.clear();
				}
			}
			return true;
		}
		return true;
	}

	int64_t DatagramSocketSSLClient::getBytesRead() noexcept {
		return this->bytesRead;
	}

	std::string HttpSSLClient::soundcloudCertPathStatic{};
	std::string HttpSSLClient::defaultCertPathStatic{};
	std::string HttpSSLClient::googleCertPathStatic{};
	std::mutex HttpSSLClient::theMutex{};
}
