<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * A class that implements a mountpoint.
 * @ingroup api
 */
class MountPoint {
	private $dir;

	/**
	 * Constructor
	 * @param dir Path to the directory, e.g.
	 *   <ul>
	 *   \li /srv/78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li /srv/7A48-BA97 (FAT)
	 *   \li /srv/2ED43920D438EC29 (NTFS)
	 *   </ul>
	 */
	public function __construct($dir) {
		$this->dir = $dir;
	}

	/**
	 * Get the directory path.
	 * @return The directory path.
	 */
	public function getPath() {
		return $this->dir;
	}

	/**
	 * Set the directory path, e.g.
	 *   <ul>
	 *   \li /srv/78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li /srv/7A48-BA97 (FAT)
	 *   \li /srv/2ED43920D438EC29 (NTFS)
	 *   </ul>
	 * @return None.
	 */
	public function setPath($dir) {
		$this->dir = $dir;
	}

	/**
	 * Check whether the directory exists.
	 * @return Returns TRUE if the directory exists, otherwise FALSE.
	 */
	public function exists() {
		if (FALSE == file_exists($this->getPath()))
			return FALSE;
		if (FALSE === is_dir($this->getPath())) {
			throw new \OMV\Exception("The file '%s' is no directory.",
				$this->getPath());
		}
		return TRUE;
	}

	/**
	 * Create the mountpoint of the file system.
	 * @param mode Set the file mode (as in chmod), e.g. '0755'.
	 *   Defaults to '0700'.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function create($mode = "0700") {
		// Exit immediatelly if the directory already exists.
		if (TRUE === $this->exists())
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--parents";
		$cmdArgs[] = sprintf("--mode=%s", $mode);
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("mkdir", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unlink the mountpoint of the file system.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function unlink($force = TRUE) {
		// Exit immediatelly if the directory doesn't exists.
		if (FALSE === $this->exists())
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--recursive";
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("rm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * See if the directory is a mountpoint.
	 * @return TRUE if the directory is a mountpoint, FALSE if not.
	 * @throw \OMV\ExecException
	 */
	public function isMountPoint() {
		$cmdArgs = [];
		$cmdArgs[] = "-q";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("mountpoint", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		return ($exitStatus == 0) ? TRUE : FALSE;
		/*
		if (!is_dir($this->getPath()))
			return FALSE;
		if (FALSE === ($stat = stat($this->getPath())))
			return FALSE;
		if (FALSE === ($stat2 = stat(sprintf("%s/..", $this->getPath()))))
			return FALSE;
		return (($stat.dev != $stat2.dev) || (($stat.dev == $stat2.dev) &&
			($stat.ino == $stat2.ino))) ? TRUE : FALSE;
		*/
	}

	/**
	 * Check if the associated mountpoint is mounted.
	 * @return TRUE if the mountpoint is mounted, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function isMounted() {
		// If the directory is a mountpoint, then we can assume that the
		// associated filesystem is mounted.
		return $this->isMountPoint();
	}

	/**
	 * Mount the this mountpoint.
	 * @param options An array or comma seperated string of additional mount
	 *   options. Defaults to "".
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function mount($options = "") {
		$cmdArgs = [];
		$cmdArgs[] = "-v";
		if (!empty($options)) {
			if (!is_array($options))
				$options = array($options);
			$cmdArgs[] = sprintf("-o %s", implode(",", $options));
		}
		$cmdArgs[] = sprintf("--target %s", escapeshellarg($this->getPath()));
		$cmd = new \OMV\System\Process("mount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unmount the this mountpoint.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = [];
		$cmdArgs[] = "-v";
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("umount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get the mountpoint path where the file system should be mounted to.
	 * Note, this path is OMV specific: /srv/<FSUUID>.
	 * @param id string The device file or UUID of the file system, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li /dev/sda2
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-id/wwn-0x5000cca211cc703c-part1
	 *   \li /dev/disk/by-id/md-name-vmpc01:data
	 *   \li /dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9
	 *   \li /dev/disk/by-label/xx\x20yy
	 *   </ul>
	 * @return The path where to mount the file system, e.g.
	 *   /srv/85732966-949a-4d8b-87d7-d7e6681f787e or
	 *   /srv/dev-disk-by-id-wwn-0x5000cca211cc703c-part1 or
	 *   /srv/dev-disk-by-id-md-name-vmpc01-data.
	 */
	public static function buildPath($id) {
		if (TRUE === is_devicefile($id)) {
			$id = unescape_path($id);
			$id = str_replace(["/", ":"], "-", trim($id, "/"));
		}
		return build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
			"OMV_MOUNT_DIR"), $id);
	}
}
