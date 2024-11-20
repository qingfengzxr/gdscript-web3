// SPDX-License-Identifier: MIT
pragma solidity ^0.7.6;
pragma experimental ABIEncoderV2;

contract Test {
    struct S {
        uint32 a;
        uint[] b;
        T[] c;
        int256[3] d;
    }

    struct C {
        string[] t;
    }

    struct T {
        C[] x;
        bytes10 y;
    }

    mapping (address=>uint256) balance;

    function f(S memory s, T memory t, uint u, address user, bytes10 b10) public pure {
        // This is a pure function, it does not modify the state nor read the state
    }

    function g() public pure returns (S memory, T memory, address, uint) {
        // Initialize S struct
        S memory s;
        s.a = 1;
        s.b = new uint[](2);
        s.b[0] = 2;
        s.b[1] = 3;
        s.c = new T[](1);
        s.c[0].x = new C[](1);
        s.c[0].x[0].t = new string[](2);
        s.c[0].x[0].t[0] = "STRING_TEST"; // Uppercase string
        s.c[0].x[0].t[1] = "string_test"; // Lowercase string
        s.c[0].y = bytes10(0x04000000000000000000); // 4 in bytes10
        s.d[0] = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF; // Large number 1
        s.d[1] = 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE; // Large number 2
        s.d[2] = ~int256(0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF); // Large number 3 (negative number)

        // Initialize T struct
        T memory t;
        t.x = new C[](1);
        t.x[0].t = new string[](2);
        t.x[0].t[0] = "STRING_test"; // Mixed case string
        t.x[0].t[1] = "STRING_TEST_MORE_THAN_32_BYTES_abcdefghijklmnopqrstuvwxyz_0000000111111222222"; // String longer than 32 bytes
        t.y = bytes10(0x08000000000000000000); // 8 in bytes10

        // Initialize address
        address addr = address(0x8eee12Bd33Ec72a277ffA9ddF246759878589D3b);

        // Initialize uint
        uint u = 9;

        return (s, t, addr, u);
    }

    function h(address user) public returns (uint256) {
        balance[user] += 1;
        return balance[user];
    }
}