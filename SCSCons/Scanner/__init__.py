# MIT License
#
# Copyright The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""The Scanner package for the SCons software construction utility."""

import re

import SCons.Node.FS
import SCons.PathList
import SCons.Util


class _Null:
    pass

# This is used instead of None as a default argument value so None can be
# used as an actual argument value.
_null = _Null

def Scanner(function, *args, **kwargs):
    """Factory function to create a Scanner Object.

    Creates the appropriate Scanner based on the type of "function".

    TODO:  Deprecate this some day.  We've moved the functionality
    inside the ScannerBase class and really don't need this factory function
    any more.  It was, however, used by some of our Tool modules, so
    the call probably ended up in various people's custom modules
    patterned on SCons code.

    """
    if SCons.Util.is_Dict(function):
        return Selector(function, *args, **kwargs)

    return ScannerBase(function, *args, **kwargs)


class FindPathDirs:
    """Class to bind a specific E{*}PATH variable name to a function that
    will return all of the E{*}path directories.
    """
    def __init__(self, variable):
        self.variable = variable

    def __call__(self, env, dir=None, target=None, source=None, argument=None):
        try:
            path = env[self.variable]
        except KeyError:
            return ()

        dir = dir or env.fs._cwd
        path = SCons.PathList.PathList(path).subst_path(env, target, source)
        return tuple(dir.Rfindalldirs(path))


class ScannerBase:
    """Base class for dependency scanners.

    Implements straightforward, single-pass scanning of a single file.

    A Scanner is usually set up with a scanner function (and optionally
    a path function), but can also be a kind of dispatcher which
    passes control to other Scanners.

    A scanner function takes three arguments: a Node to scan for
    dependencies, the construction environment to use, and an optional
    tuple of paths (as generated by the optional path function).
    It must return a list containing the Nodes for all the direct
    dependencies of the file.

    The optional path function is called to return paths that can be
    searched for implicit dependency files. It takes five arguments:
    a construction environment, a Node for the directory containing
    the SConscript file that defined the primary target, a list of
    target nodes, a list of source nodes, and the optional argument
    for this instance.

    Examples::

        s = Scanner(my_scanner_function)
        s = Scanner(function=my_scanner_function)
        s = Scanner(function=my_scanner_function, argument='foo')

    Args:
        function: either a scanner function taking two or three arguments
          and returning a list of File Nodes; or a mapping of keys to
          other Scanner objects.

        name: an optional name for identifying this scanner object
          (defaults to "NONE").

        argument: an optional argument that will be passed to both
          *function* and *path_function*.

        skeys: an optional list argument that can be used
          to determine if this scanner can be used for a given Node.
          In the case of File nodes, for example, the *skeys*
          would be file suffixes.

        path_function: an optional function which returns a tuple
          of the directories that can be searched for implicit
          dependency files.  May also return a callable which
          is called with no args and returns the tuple (supporting
          Bindable class).

        node_class: optional class of Nodes which this scan will return.
          If not specified, defaults to :class:`SCons.Node.FS.Base`.
          If *node_class* is ``None``, then this scanner will not enforce
          any Node conversion and will return the raw results from *function*.

        node_factory: optional factory function to be called to
          translate the raw results returned by *function*
          into the expected *node_class* objects.

        scan_check: optional function to be called to first check whether
          this node really needs to be scanned.

        recursive: optional specifier of whether this scanner should be
          invoked recursively on all of the implicit dependencies it returns
          (for example `#include` lines in C source files, which may refer
          to header files which should themselves be scanned).
          May be a callable, which will be called to filter
          the list of nodes found to select a subset for recursive
          scanning (the canonical example being only recursively
          scanning subdirectories within a directory). The default
          is to not do recursive scanning.
    """

    def __init__(
        self,
        function,
        name="NONE",
        argument=_null,
        skeys=_null,
        path_function=None,
        # Node.FS.Base so that, by default, it's okay for a
        # scanner to return a Dir, File or Entry.
        node_class=SCons.Node.FS.Base,
        node_factory=None,
        scan_check=None,
        recursive=None,
    ):
        """Construct a new scanner object given a scanner function."""
        # Note: this class could easily work with scanner functions that take
        # something other than a filename as an argument (e.g. a database
        # node) and a dependencies list that aren't file names. All that
        # would need to be changed is the documentation.

        self.function = function
        self.path_function = path_function
        self.name = name
        self.argument = argument

        if skeys is _null:
            if SCons.Util.is_Dict(function):
                skeys = list(function.keys())
            else:
                skeys = []
        self.skeys = skeys

        self.node_class = node_class
        self.node_factory = node_factory
        self.scan_check = scan_check
        if callable(recursive):
            self.recurse_nodes = recursive
        elif recursive:
            self.recurse_nodes = self._recurse_all_nodes
        else:
            self.recurse_nodes = self._recurse_no_nodes

    def path(self, env, dir=None, target=None, source=None):
        if not self.path_function:
            return ()

        if self.argument is not _null:
            return self.path_function(env, dir, target, source, self.argument)

        return self.path_function(env, dir, target, source)

    def __call__(self, node, env, path=()) -> list:
        """Scans a single object.

        Args:
          node: the node that will be passed to the scanner function
          env: the environment that will be passed to the scanner function.
          path: tuple of paths from the `path_function`

        Returns:
          A list of direct dependency nodes for the specified node.

        """
        if self.scan_check and not self.scan_check(node, env):
            return []

        # here we may morph into a different Scanner instance:
        self = self.select(node)  # pylint: disable=self-cls-assignment

        if self.argument is not _null:
            node_list = self.function(node, env, path, self.argument)
        else:
            node_list = self.function(node, env, path)

        kw = {}
        if hasattr(node, 'dir'):
            kw['directory'] = node.dir
        conv = env.get_factory(self.node_factory)
        cls = self.node_class
        nl = [conv(n, **kw) if cls and not isinstance(n, cls) else n for n in node_list]
        return nl

    def __eq__(self, other):
        try:
            return self.__dict__ == other.__dict__
        except AttributeError:
            # other probably doesn't have a __dict__
            return self.__dict__ == other

    def __hash__(self):
        return id(self)

    def __str__(self):
        return self.name

    def add_skey(self, skey):
        """Add a skey to the list of skeys"""
        self.skeys.append(skey)

    def get_skeys(self, env=None):
        if env and SCons.Util.is_String(self.skeys):
            return env.subst_list(self.skeys)[0]
        return self.skeys

    def select(self, node):
        if SCons.Util.is_Dict(self.function):
            key = node.scanner_key()
            try:
                return self.function[key]
            except KeyError:
                return None
        else:
            return self

    @staticmethod
    def _recurse_all_nodes(nodes):
        return nodes

    @staticmethod
    def _recurse_no_nodes(nodes):
        return []

    # recurse_nodes = _recurse_no_nodes

    def add_scanner(self, skey, scanner):
        self.function[skey] = scanner
        self.add_skey(skey)


# keep the old name for a while in case external users are using.
# there are no more internal uses of this class by the name "Base"
Base = ScannerBase


class Selector(ScannerBase):
    """
    A class for selecting a more specific scanner based on the
    :func:`scanner_key` (suffix) for a specific Node.

    TODO:  This functionality has been moved into the inner workings of
    the ScannerBase class, and this class will be deprecated at some point.
    (It was never exposed directly as part of the public interface,
    although it is used by the :func:`Scanner` factory function that was
    used by various Tool modules and therefore was likely a template
    for custom modules that may be out there.)
    """
    def __init__(self, mapping, *args, **kwargs):
        super().__init__(None, *args, **kwargs)
        self.mapping = mapping
        self.skeys = list(mapping.keys())

    def __call__(self, node, env, path=()):
        return self.select(node)(node, env, path)

    def select(self, node):
        try:
            return self.mapping[node.scanner_key()]
        except KeyError:
            return None

    def add_scanner(self, skey, scanner):
        self.mapping[skey] = scanner
        self.add_skey(skey)


class Current(ScannerBase):
    """
    A class for scanning files that are source files (have no builder)
    or are derived files and are current (which implies that they exist,
    either locally or in a repository).
    """

    def __init__(self, *args, **kwargs):
        def current_check(node, env):
            return not node.has_builder() or node.is_up_to_date()

        kwargs['scan_check'] = current_check
        super().__init__(*args, **kwargs)

class Classic(Current):
    """
    A Scanner subclass to contain the common logic for classic CPP-style
    include scanning, but which can be customized to use different
    regular expressions to find the includes.

    Note that in order for this to work "out of the box" (without
    overriding the :meth:`find_include` and :meth:`sort_key1` methods),
    the regular expression passed to the constructor must return the
    name of the include file in group 0.
    """

    def __init__(self, name, suffixes, path_variable, regex, *args, **kwargs):
        self.cre = re.compile(regex, re.M)

        def _scan(node, _, path=(), self=self):
            node = node.rfile()
            if not node.exists():
                return []
            return self.scan(node, path)

        kwargs['function'] = _scan
        kwargs['path_function'] = FindPathDirs(path_variable)

        # Allow recursive to propagate if child class specifies.
        # In this case resource scanner needs to specify a filter on which files
        # get recursively processed.  Previously was hardcoded to 1 instead of
        # defaulted to 1.
        kwargs['recursive'] = kwargs.get('recursive', True)
        kwargs['skeys'] = suffixes
        kwargs['name'] = name

        super().__init__(*args, **kwargs)

    @staticmethod
    def find_include(include, source_dir, path):
        n = SCons.Node.FS.find_file(include, (source_dir,) + tuple(path))
        return n, include

    @staticmethod
    def sort_key(include):
        return SCons.Node.FS._my_normcase(include)

    def find_include_names(self, node):
        return self.cre.findall(node.get_text_contents())

    def scan(self, node, path=()):
        # cache the includes list in node so we only scan it once:
        if node.includes is not None:
            includes = node.includes
        else:
            includes = self.find_include_names(node)
            # Intern the names of the include files. Saves some memory
            # if the same header is included many times.
            node.includes = list(map(SCons.Util.silent_intern, includes))

        # This is a hand-coded DSU (decorate-sort-undecorate, or
        # Schwartzian transform) pattern.  The sort key is the raw name
        # of the file as specified on the #include line (including the
        # " or <, since that may affect what file is found), which lets
        # us keep the sort order constant regardless of whether the file
        # is actually found in a Repository or locally.
        nodes = []
        source_dir = node.get_dir()
        if callable(path):
            path = path()
        for include in includes:
            n, i = self.find_include(include, source_dir, path)

            if n is None:
                SCons.Warnings.warn(
                    SCons.Warnings.DependencyWarning,
                    "No dependency generated for file: %s "
                    "(included from: %s) -- file not found" % (i, node),
                )
            else:
                nodes.append((self.sort_key(include), n))

        return [pair[1] for pair in sorted(nodes)]

class ClassicCPP(Classic):
    """
    A Classic Scanner subclass which takes into account the type of
    bracketing used to include the file, and uses classic CPP rules
    for searching for the files based on the bracketing.

    Note that in order for this to work, the regular expression passed
    to the constructor must return the leading bracket in group 0, and
    the contained filename in group 1.
    """
    def find_include(self, include, source_dir, path):
        include = list(map(SCons.Util.to_str, include))
        if include[0] == '"':
            paths = (source_dir,) + tuple(path)
        else:
            paths = tuple(path) + (source_dir,)

        n = SCons.Node.FS.find_file(include[1], paths)
        i = SCons.Util.silent_intern(include[1])
        return n, i

    def sort_key(self, include):
        return SCons.Node.FS._my_normcase(' '.join(include))

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
