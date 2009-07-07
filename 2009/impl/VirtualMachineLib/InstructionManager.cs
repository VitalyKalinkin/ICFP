using System;
using System.Collections.Generic;
using System.IO;

namespace ICFP2009.VirtualMachineLib
{
    internal class InstructionManager
    {
        private readonly Int32[] _instructions;
        private Int16 _currentIndex;
        private bool _statusRegister;

        public InstructionManager(List<Int32> instructions)
        {
            _instructions = instructions.ToArray();
        }

        public void RunOneStep()
        {
            MemoryManager memory = VirtualMachine.Instance.Memory;
            PortManager ports = VirtualMachine.Instance.Ports;

            _currentIndex = -1;

            while (_currentIndex + 1 < _instructions.Length)
            {
                ++_currentIndex;

                Int32 currentInstruction = _instructions[_currentIndex];

                // � 31 �� 28 ����. 4 ����.
                var opCode = (byte) ((currentInstruction & 0xF0000000) >> 28);

                // ������ S-Type
                if (opCode == 0)
                {
                    // � 27 �� 24 ����. 4 ����.
                    var sTypeOpCode = (byte) ((currentInstruction & 0x0F000000) >> 24);

                    // C 13 �� 0 ����. 14 ���.
                    var r1 = (Int16) (currentInstruction & 0x00003FFF);

                    switch (sTypeOpCode)
                    {
                            // Noop. ������ �� ������.
                        case 0x00:
                            //Console.WriteLine("Noop");
                            break;

                            // Cmpz. �������� ���������.
                        case 0x01:
                            // � 23 �� 21 ����. 10 ���.
                            var immediate = (byte) ((currentInstruction & 0x00E00000) >> 21);

                            // ��� ���������.
                            switch (immediate)
                            {
                                    // LTZ. ������ ���.
                                case 0x00:
                                    _statusRegister = memory[r1] < 0.0;
                                    break;

                                    // LEZ. ������ ��� �����.
                                case 0x01:
                                    _statusRegister = memory[r1] < 0.0 || memory[r1] == 0.0;
                                    break;

                                    // EQZ. �����.
                                case 0x02:
                                    _statusRegister = memory[r1] == 0;
                                    break;

                                    // GEZ. ������ ��� �����.
                                case 0x03:
                                    _statusRegister = memory[r1] > 0.0 && memory[r1] == 0.0;
                                    break;

                                    // GTZ. ������.
                                case 0x04:
                                    _statusRegister = memory[r1] > 0.0;
                                    break;

                                default:
                                    throw new InvalidDataException(
                                        string.Format("Immediate \"0x{0:x}\" does not exists.", immediate));
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

                        default:
                            throw new InvalidDataException(
                                string.Format("S-Type op code \"0x{0:x}\" does not exists.", sTypeOpCode));
                    }
                }
                    // ������ D-Type
                else
                {
                    // C 27 �� 14 ����. 14 ���.
                    var r1 = (Int16) ((currentInstruction & 0x0FFFC000) >> 14);

                    // C 13 �� 0 ����. 14 ���.
                    var r2 = (Int16)  (currentInstruction & 0x00003FFF);

                    switch (opCode)
                    {
                            // Add. ��������.
                        case 0x01:
                            memory[_currentIndex] = memory[r1] + memory[r2];
                           // Console.WriteLine("{0} + {1} == {2}", memory[r1], memory[r2], memory[_currentIndex]);
                            break;

                            // Sub. ���������.
                        case 0x02:
                            memory[_currentIndex] = memory[r1] - memory[r2];
                            //Console.WriteLine("{0} - {1} == {2}", memory[r1], memory[r2], memory[_currentIndex]);
                            break;

                            // Mult. ���������.
                        case 0x03:
                            memory[_currentIndex] = memory[r1] * memory[r2];
                            break;

                            // Div. �������.
                        case 0x04:
                            if (memory[r2] == 0)
                                memory[_currentIndex] = 0.0;
                            else
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

                        default:
                            throw new InvalidDataException(
                                string.Format("D-Type op code \"0x{0:x}\" does not exists.", opCode));
                    }
                }
            }
        }
    }
}