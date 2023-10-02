import re
from pathlib import Path
from pprint import pformat

from typing_extensions import TypeAlias

INDEX = "index.md"
NAV_KEY = "nav"
FILENAME_PATTERN = re.compile(r"[a-z0-9-/.]+")

DataList: TypeAlias = list[str | dict]
Data: TypeAlias = str | DataList
Content: TypeAlias = dict[str, Data]


class NavigationConfigError(ValueError):
    ...


class MissingIndexPage(NavigationConfigError):
    ...


class InvalidFilename(NavigationConfigError):
    ...


class SectionNotFoundError(NavigationConfigError):
    ...


def validate_navigation(config: dict) -> None:
    _check_conventions({NAV_KEY: config[NAV_KEY]})


def _get_section(navigation: DataList, section_title: str) -> DataList:
    for item in navigation:
        if (
            isinstance(item, dict)
            and len(item.keys()) == 1
            and list(item.keys())[0] == section_title
        ):
            return item[section_title]
    raise SectionNotFoundError(
        f"The section: {section_title!r} was not found in the navigation."
    )


def _check_conventions(content: Content) -> None:
    section_name = _get_section_name(content)
    data = content[section_name]

    # Forgive a missing index for the root "nav"
    if section_name != NAV_KEY:
        _check_missing_index(data)

    if isinstance(data, str):
        _check_filename(data)
        return None

    if isinstance(data, list):
        for elem in data:
            if isinstance(elem, str):
                _check_filename(elem)
            elif isinstance(elem, dict):
                _check_conventions(elem)
        return None

    raise NavigationConfigError(
        f"Received data of an unexpected type (not a list or str):\n{pformat(data)}"
    )


def _get_section_name(content: Content) -> str:
    keys = list(content)
    if not len(keys) == 1:
        raise NavigationConfigError(f"Expected one section but found {keys}")
    return keys[0]


def _check_missing_index(content_value: Data) -> None:
    if isinstance(content_value, list):
        if len(content_value) > 1:
            # This is a parent section - require index page
            first_page = content_value[0]
            if not (isinstance(first_page, str) and Path(first_page).name == INDEX):
                raise MissingIndexPage(
                    "The first page of a parent section is expected to be an index "
                    f"(`{INDEX}` file),\n"
                    "but it was not found in:\n"
                    f"{pformat(content_value)}"
                )


def _check_filename(filename: str) -> None:
    if FILENAME_PATTERN.fullmatch(filename) is None:
        raise InvalidFilename(
            "The filename:\n"
            f"{filename}\n"
            "includes characters that do not comply with the convention.\n"
            "Please use only lowercase letters (a-z), numbers (0-9), dots (.) and "
            "hyphens (-)."
        )
