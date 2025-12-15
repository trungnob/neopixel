// NetworkManager.swift
import Foundation
import SwiftUI

final class NetworkManager: NSObject, ObservableObject {
    static let shared = NetworkManager()
    override private init() {
        super.init()
    }

    // MARK: - Discovery
    private var browser: NetServiceBrowser?
    @Published var services: [DiscoveredService] = []
    @Published var selected: DiscoveredService?

    struct DiscoveredService: Hashable {
        let name: String
        let host: String   // IPv4 address
    }

    func startBrowsing() {
        browser?.stop()
        browser = NetServiceBrowser()
        browser?.delegate = self
        browser?.searchForServices(ofType: "_neopixel._tcp.", inDomain: "local.")
    }

    func select(service: DiscoveredService) {
        selected = service
    }

    func setManualIP(_ ip: String) {
        guard !ip.isEmpty else { return }
        selected = DiscoveredService(name: "Manual", host: ip)
    }

    // MARK: - LED matrix handling
    private let rows = 32
    private let cols = 32
    private var ledMatrix = Array(repeating: Color.black, count: 32 * 32)

    func colorAt(row: Int, col: Int) -> Color {
        ledMatrix[row * cols + col]
    }

    func togglePixel(row: Int, col: Int) {
        let idx = row * cols + col
        ledMatrix[idx] = ledMatrix[idx] == .black ? .red : .black
        sendPixel(row: row, col: col, color: ledMatrix[idx])
    }

    private func sendPixel(row: Int, col: Int, color: Color) {
        guard let host = selected?.host else { return }
        let rgb = rgbFrom(color: color)
        let path = "/api/setPixel?row=\(row)&col=\(col)&r=\(rgb.r)&g=\(rgb.g)&b=\(rgb.b)"
        request(host: host, path: path)
    }

    private func rgbFrom(color: Color) -> (r: Int, g: Int, b: Int) {
        if color == .red { return (255,0,0) }
        return (0,0,0)
    }

    private func request(host: String, path: String) {
        guard let url = URL(string: "http://\(host)\(path)") else { return }
        var req = URLRequest(url: url)
        req.httpMethod = "GET"
        URLSession.shared.dataTask(with: req) { _, _, _ in }.resume()
    }
}

// MARK: - NetServiceBrowserDelegate & NetServiceDelegate
extension NetworkManager: NetServiceBrowserDelegate, NetServiceDelegate {
    func netServiceBrowser(_ browser: NetServiceBrowser, didFind service: NetService, moreComing: Bool) {
        service.delegate = self
        service.resolve(withTimeout: 5)
    }

    func netServiceDidResolveAddress(_ sender: NetService) {
        guard let addresses = sender.addresses else { return }
        for data in addresses {
            var addr = sockaddr_in()
            (data as NSData).getBytes(&addr, length: MemoryLayout<sockaddr_in>.size)
            if addr.sin_family == sa_family_t(AF_INET) {
                let ip = String(cString: inet_ntoa(addr.sin_addr))
                let discovered = DiscoveredService(name: sender.name, host: ip)
                DispatchQueue.main.async {
                    if !self.services.contains(discovered) {
                        self.services.append(discovered)
                    }
                }
                break
            }
        }
    }

    func netServiceBrowser(_ browser: NetServiceBrowser, didRemove service: NetService, moreComing: Bool) {
        DispatchQueue.main.async {
            self.services.removeAll { $0.name == service.name }
        }
    }
}
