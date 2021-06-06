#include <QCoreApplication>
#include <QCommandLineParser>
#include <QNetworkInterface>
#include <QHostAddress>

#include "Utils.h"

#include "../plugin-macros.generated.h"

#include <QDebug>

std::string Utils::Platform::GetLocalAddress()
{
	std::vector<QString> validAddresses;
	for (auto address : QNetworkInterface::allAddresses()) {
		// Exclude addresses which won't work
		if (address == QHostAddress::LocalHost)
			continue;
		else if (address == QHostAddress::LocalHostIPv6)
			continue;
		else if (address.isLoopback())
			continue;
		else if (address.isLinkLocal())
			continue;
		else if (address.isNull())
			continue;

		validAddresses.push_back(address.toString());
	}

	// Return early if no valid addresses were found
	if (validAddresses.size() == 0)
		return "0.0.0.0";

	std::vector<std::pair<QString, uint8_t>> preferredAddresses;
	for (auto address : validAddresses) {
		// Attribute a priority (0 is best) to the address to choose the best picks
		if (address.startsWith("192.168.1") || address.startsWith("192.168.0")) { // Prefer common consumer router network prefixes
			preferredAddresses.push_back(std::make_pair(address, 0));
		} else if (address.startsWith("172.16")) { // Slightly less common consumer router network prefixes
			preferredAddresses.push_back(std::make_pair(address, 1));
		} else if (address.startsWith("10.")) { // Even less common consumer router network prefixes
			preferredAddresses.push_back(std::make_pair(address, 2));
		} else { // Set all other addresses to equal priority
			preferredAddresses.push_back(std::make_pair(address, 255));
		}
	}

	// Sort by priority
	std::sort(preferredAddresses.begin(), preferredAddresses.end(), [=](std::pair<QString, uint8_t> a, std::pair<QString, uint8_t> b) {
		return a.second < b.second;
	});

	// Return highest priority address
	return preferredAddresses[0].first.toStdString();
}

QString Utils::Platform::GetCommandLineArgument(QString arg)
{
	QCommandLineParser parser;
	QCommandLineOption cmdlineOption(arg, arg, arg, "");
	parser.addOption(cmdlineOption);
	parser.parse(QCoreApplication::arguments());

	if (!parser.isSet(cmdlineOption))
		return "";

	return parser.value(cmdlineOption);
}

bool Utils::Platform::GetCommandLineFlagSet(QString arg)
{
	QCommandLineParser parser;
	QCommandLineOption cmdlineOption(arg, arg, arg, "");
	parser.addOption(cmdlineOption);
	parser.parse(QCoreApplication::arguments());

	return parser.isSet(cmdlineOption);
}
