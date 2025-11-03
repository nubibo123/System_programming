#!/bin/bash
# Script to monitor shared memory in real-time

echo "╔════════════════════════════════════════════════════════╗"
echo "║     SHARED MEMORY MONITOR - External View             ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Check if shared memory exists
echo "📊 SYSTEM SHARED MEMORY SEGMENTS:"
echo "════════════════════════════════════════════════════════"
ipcs -m | head -1
ipcs -m | grep -E "1234|mail_system|lequangchinh" || echo "   (No mail system shared memory found)"
echo ""

# Show detailed info
echo "📋 DETAILED INFORMATION:"
echo "────────────────────────────────────────────────────────"
ipcs -m -i $(ipcs -m | grep 666 | awk '{print $2}' | head -1) 2>/dev/null || echo "   No accessible shared memory"
echo ""

echo "💡 TIP: Run './mail_system' to create shared memory"
echo "💡 TIP: Use option 6 in the program to see live data"
echo ""

# Continuous monitoring option
if [ "$1" == "-w" ] || [ "$1" == "--watch" ]; then
    echo "🔄 Monitoring mode (Ctrl+C to exit)..."
    echo "════════════════════════════════════════════════════════"
    watch -n 2 "ipcs -m | grep -E 'key|666'"
fi
