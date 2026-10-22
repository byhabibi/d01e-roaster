#!/usr/bin/env python3
"""Extract Nextion HMI event code to directory tree (read-only parser)."""
import sys
import os
from os.path import basename, exists, realpath
from os import mkdir
from re import compile


class Attr:
    def __init__(self, num):
        self.num = num
        self.objname = ""
        self.typename = ""
        self.codesload = ""
        self.codesloadend = ""
        self.codesdown = ""
        self.codesup = ""
        self.codesunload = ""
        self.codestimer = ""
        self.codesplayend = ""
        self.page_attr = None

    def create_files_inside(self, dir_name):
        if self.is_some_non_empty_event():
            if self.page_attr is None:
                print(f"Probably false positive, {self.objname} has not page_attr")
                return
            attr_dir = dir_name + "/" + self.page_attr.objname
            if not exists(attr_dir):
                mkdir(attr_dir)
            attr_fname_prefix = attr_dir + "/" + self.objname + "."
            self.create_file(attr_fname_prefix + "init_pre.txt", self.codesload)
            self.create_file(attr_fname_prefix + "init_post.txt", self.codesloadend)
            self.create_file(attr_fname_prefix + "touch_press.txt", self.codesdown)
            self.create_file(attr_fname_prefix + "touch_release.txt", self.codesup)
            self.create_file(attr_fname_prefix + "page_exit.txt", self.codesunload)
            self.create_file(attr_fname_prefix + "timer.txt", self.codestimer)
            self.create_file(attr_fname_prefix + "play_end.txt", self.codesplayend)

    def is_some_non_empty_event(self):
        return any(
            (
                self.codesload,
                self.codesloadend,
                self.codesdown,
                self.codesup,
                self.codesunload,
                self.codestimer,
                self.codesplayend,
            )
        )

    def create_file(self, fname, code):
        if code:
            with open(fname, "wb") as f:
                f.write(code.encode())
                f.write(b"\n")


def get_event_code(i, strings, line, regex):
    event_code = ""
    strings_jump = 0
    check = regex.match(line)
    if check:
        strings_jump = int(check[1])
        if strings_jump > 0:
            event_code = "\n".join(strings[i + 1 : i + 1 + strings_jump])
    return event_code, strings_jump


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {basename(sys.argv[0])} project.hmi output_dir")
        sys.exit(1)

    with open(sys.argv[1], "rb") as f:
        data = f.read()

    strings = []
    line = ""
    i = 0
    printable = False
    for c in data:
        if c == 0:
            i = i + 1 if i else 0
            if not printable:
                continue
            ch = chr(c)
            printable = ch.isprintable()
            if printable:
                if i == 3:
                    strings[-1] = strings[-1][:-1]
                    i = 0
                line += ch
            else:
                i = 0
                if len(line) > 0:
                    strings.append(line)
                i = 1
                line = ""

    attrs = []
    attr = None
    strings_jump = 0
    attr_regex = compile(r"^att-(\d+)$")
    codesload_regex = compile(r"^codesload-(\d+)$")
    codesloadend_regex = compile(r"^codesloadend-(\d+)$")
    codesdown_regex = compile(r"^codesdown-(\d+)$")
    codesup_regex = compile(r"^codesup-(\d+)$")
    codesunload_regex = compile(r"^codesunload-(\d+)$")
    codestimer_regex = compile(r"^codestimer-(\d+)$")
    codesplayend_regex = compile(r"^codesplayend-(\d+)$")
    objname_next = False
    type_next = False
    current_page_attr = None

    for i, line in enumerate(strings):
        if strings_jump:
            strings_jump -= 1
            continue
        if len(line) == 0:
            continue

        attr_check = attr_regex.match(line)
        if attr_check:
            if attr is not None:
                attr.page_attr = current_page_attr
                attrs.append(attr)
            attr = Attr(attr_check[1])
            continue

        for regex, field in [
            (codesload_regex, "codesload"),
            (codesloadend_regex, "codesloadend"),
            (codesdown_regex, "codesdown"),
            (codesup_regex, "codesup"),
            (codesunload_regex, "codesunload"),
            (codestimer_regex, "codestimer"),
            (codesplayend_regex, "codesplayend"),
        ]:
            event_code, strings_jump = get_event_code(i, strings, line, regex)
            if strings_jump:
                setattr(attr, field, event_code)
                break
        else:
            if objname_next:
                if line != "vscope":
                    if attr.is_some_non_empty_event():
                        attr.page_attr = current_page_attr
                        attrs.append(attr)
                    attr = Attr(0)
                    attr.objname = line
                objname_next = False
            elif line == "objname":
                objname_next = True
            elif type_next:
                attr.typename = line
                if line == "y":
                    current_page_attr = attr
                type_next = False
            elif line == "type":
                type_next = True

    base_dir = sys.argv[2]
    if not exists(base_dir):
        mkdir(base_dir)
    for a in attrs:
        a.create_files_inside(base_dir)
    print(realpath(base_dir))


if __name__ == "__main__":
    main()
