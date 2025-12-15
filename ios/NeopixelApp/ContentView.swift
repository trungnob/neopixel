// ContentView.swift
import SwiftUI

struct ContentView: View {
    @ObservedObject var network = NetworkManager.shared
    @State private var manualIP = ""

    var body: some View {
        VStack {
            // Device discovery list
            List {
                Section(header: Text("Discovered ESP8266 Devices")) {
                    ForEach(network.services, id: \.self) { service in
                        Button(action: {
                            network.select(service: service)
                        }) {
                            Text("\(service.name) (\(service.host))")
                                .foregroundColor(network.selected?.host == service.host ? .blue : .primary)
                        }
                    }
                }
                Section(header: Text("Manual IP")) {
                    HStack {
                        TextField("e.g. 192.168.1.130", text: $manualIP)
                        Button("Use") {
                            network.setManualIP(manualIP)
                        }
                    }
                }
            }
            .frame(height: 200)

            // Pattern grid
            PatternGrid()
                .environmentObject(network)

            Spacer()
        }
        .padding()
        .onAppear {
            network.startBrowsing()
        }
    }
}
