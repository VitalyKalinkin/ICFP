using System;
using System.Collections.Generic;

namespace ICFP2009.VirtualMachineLib
{
    internal class InstructionManager
    {
        private const double _epsilon = 1e-15;
        private readonly Int32[] _instructions;
        private Int16 _currentIndex;
        private bool _statusRegister;

        public InstructionManager(List<Int32> instructions)
        {
            _instructions = instructions.ToArray();
        }

        public void StartMainLoop()
        {
            MemoryManager memory = VirtualMachine.Instance.Memory;
            PortManager ports = VirtualMachine.Instance.Ports;

            while (true)
            {
                Int32 currentInstruction = _instructions[_currentIndex];

                // � 21 �� 28 ����. 4 ����.
                var opCode = (byte) (currentInstruction & 0xF0000000 >> 28);

                // ������ S-Type
                if (opCode == 0)
                {
                    // � 27 �� 24 ����. 4 ����.
                    var sTypeOpCode = (byte) (currentInstruction & 0x0F000000 >> 24);

                    // C 13 �� 0 ����. 14 ���.
                    var r1 = (Int16) (currentInstruction & 0x000003FF);

                    switch (sTypeOpCode)
                    {
                            // Noop. ������ �� ������.
                        case 0x00:
                            break;
                            // Cmpz. �������� ���������.
                        case 0x01:
                            // � 23 �� 21 ����. 10 ���.
                            var immediate = (byte) (currentInstruction & 0x00E00000 >> 21);

                            // ��� ���������.
                            switch (immediate)
                            {
                                    // LTZ. ������ ���.
                                case 0x00:
                                    _statusRegister = memory[r1] < 0.0;
                                    break;
                                    // LEZ. ������ ��� �����.
                                case 0x01:
                                    _statusRegister = memory[r1] < 0.0 || Math.Abs(memory[r1]) < _epsilon;
                                    break;
                                    // EQZ. �����.
                                case 0x02:
                                    _statusRegister = Math.Abs(memory[r1]) < _epsilon;
                                    break;
                                    // GEZ. ������ ��� �����.
                                case 0x03:
                                    _statusRegister = memory[r1] > 0.0 || Math.Abs(memory[r1]) < _epsilon;
                                    break;
                                    // GTZ. ������.
                                case 0x04:
                                    _statusRegister = memory[r1] > 0.0;
                                    break;
                            }
                            break;
                            // Sqrt. ���������� ������.
                        case 0x02:
                            memory[_currentIndex] = Math.Sqrt(memory[r1]);
                            break;
                            // Copy. �������� �� ����� ������� ������ � ������.
                        case 0x03:
                            memory[_currentIndex] = memory[r1];
                            break;
                            // Input. ������ �������� �� �����.
                        case 0x04:
                            memory[_currentIndex] = ports.Input[r1];
                            break;
                    }
                }
                    // ������ D-Type
                else
                {
                    // C 27 �� 14 ����. 14 ���.
                    var r1 = (Int16) (currentInstruction & 0x000FFC00 >> 14);

                    // C 13 �� 0 ����. 14 ���.
                    var r2 = (Int16) (currentInstruction & 0x000003FF);

                    switch (opCode)
                    {
                            // Add. ��������.
                        case 0x01:
                            memory[_currentIndex] = memory[r1] + memory[r2];
                            break;
                            // Sub. ���������.
                        case 0x02:
                            memory[_currentIndex] = memory[r1] + memory[r2];
                            break;
                            // Mult. ���������.
                        case 0x03:
                            memory[_currentIndex] = memory[r1] * memory[r2];
                            break;
                            // Div. �������.
                        case 0x04:
                            if (Math.Abs(memory[r2]) < _epsilon)
                                memory[_currentIndex] = 0.0;

                            memory[_currentIndex] = memory[r1] / memory[r2];
                            break;
                            // Output. ����� � ����.
                        case 0x05:
                            ports.Output[r1] = memory[r2];
                            break;
                            // Phi. ���-�� ��������.
                        case 0x06:
                            if (_statusRegister)
                                memory[_currentIndex] = memory[r1];
                            else
                                memory[_currentIndex] = memory[r2];
                            break;
                    }
                }
            }
        }
    }
}