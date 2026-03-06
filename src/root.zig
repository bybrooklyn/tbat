const std = @import("std");
const ltd = @import("ltd");

const Parser = struct {
    data: []const u8,
    index: usize,

    const ReservedMarker = struct {};

    pub fn Reserved(comptime T: type) type {
        return struct {
            pub const marker: ReservedMarker = .{};
            pub const impersonated = T;
        };
    }

    pub fn init(data: []const u8) !@This() {
        var self = @This(){};
        self.data = data;
        return data;
    }

    pub fn section(self: @This(), offset: usize, size: usize) !@This() {
        if (offset + size > self.data.len) return error.SectionOutsideRange;
        return .{ .data = self.data[offset .. offset + size] };
    }

    fn readNumber(self: *@This(), comptime T: type) !T {
        if (self.index + @sizeOf(T) >= self.data.len) return error.DataDepleted;
        const value = std.mem.readInt(T, self.data[self.index .. self.index + @sizeOf(T)], .big);
        self.index += @sizeOf(T);
        return value;
    }

    fn readStruct(self: *@This(), comptime T: type) !T {
        if (@hasDecl(T, "marker") and @TypeOf(T.marker) == ReservedMarker) _ = try self.read(T.impersonated);
        var value: T = undefined;
        inline for (std.meta.fields(T)) |field| @field(value, field.name) = try self.read(field.type);
        return value;
    }

    pub fn read(self: *@This(), comptime T: type) !T {
        comptime switch (@typeInfo(T)) {
            .Int, .Float => return self.readNumber(T),
            .Struct => return self.readStruct(T),
            else => @compileError("Can only read integers, floating point numbers, or structs made up of the prior two"),
        };
    }
};

const Program = struct {
    const magic = "BZC2";

    pub const Header = struct {
        major: u16,
        minor: u16,
        flags: u32,
        sections: u16,
        reserved: Parser.Reserved(u16),

        pub fn init(parser: *Parser) !@This() {
            if (try parser.read() != std.mem.bytesToValue(u32, magic[0..4])) return error.MagicNumberIncorrect;
            const header = try parser.read(@This());
            if (header.sections == 0) return error.SectionCountZero;
            return header;
        }
    };

    pub const Section = struct {
        pub const Type = enum {
            module_table,
            const_pool,
            function_pool,
            code_section,
            export_table,
            import_table,
            metadata,
        };

        id: u16,
        flags: u16,
        offset: u32,
        size: u32,
        reserved: Parser.Reserved(u32),

        pub fn init(parser: *Parser) !@This() {
            const section = try parser.read(@This());
            if (section.size > 64 * 1024 * 1024) return error.SectionTooLarge;
            if (section.id >= std.meta.fields(Type)) {
                if (section.flags & 1 != 0) return error.UnknownSection;
                if (section.flags & 2 == 0) return error.UnknownSection;
            }
            return section;
        }
    };
};

pub fn bufferedPrint() !void {
    // Stdout is for the actual output of your application, for example if you
    // are implementing gzip, then only the compressed bytes should be sent to
    // stdout, not any debugging messages.
    var stdout_buffer: [1024]u8 = undefined;
    var stdout_writer = std.fs.File.stdout().writer(&stdout_buffer);
    const stdout = &stdout_writer.interface;

    try stdout.print("Run `zig build test` to run the tests.\n", .{});

    try stdout.flush(); // Don't forget to flush!
}

pub fn add(a: i32, b: i32) i32 {
    return a + b;
}

test "basic add functionality" {
    try std.testing.expect(add(3, 7) == 10);
}
