// PatternGrid.swift
import SwiftUI

struct PatternGrid: View {
    @EnvironmentObject var network: NetworkManager
    private let rows = 32
    private let cols = 32
    private let cellSize: CGFloat = 10

    var body: some View {
        VStack(spacing: 1) {
            ForEach(0..<rows, id: \.self) { r in
                HStack(spacing: 1) {
                    ForEach(0..<cols, id: \.self) { c in
                        Rectangle()
                            .fill(network.colorAt(row: r, col: c))
                            .frame(width: cellSize, height: cellSize)
                            .onTapGesture {
                                network.togglePixel(row: r, col: c)
                            }
                    }
                }
            }
        }
        .padding()
    }
}
