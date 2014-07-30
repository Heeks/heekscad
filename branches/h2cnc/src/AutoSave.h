
#pragma once

#include <wx/timer.h>

/**
	The CAutoSave class writes a backup copy of the current data model
	to a file that has a known name.  In normal use, this file will
	be deleted by the destructor of this class on the assumption that
	the operator has successfully exited and either manually saved
	or discarded their data to their own file.  If, on the other hand, a failure occurs
	then this class will not have had a chance to delete the backup
	file as part of a normal shutdown.  The next time the application
	starts up, the user will be prompted to see if they wish to use the
	backup file's contents or start again.

	This class inherits from the wxTimer class so that regular calls
	to the Notify() method are made from within the same main
	thread as all other HeeksCAD processing occurs.  This should
	avoid thread synchronisation problems.
 */
class CAutoSave : public wxTimer
{
public:
	CAutoSave(const int interval, const bool skip_recovery = false);
	~CAutoSave();

public:
	// This overloads the wxTimer::Notify() method
	void Notify();
	wxString BackupFileName() const { return(m_backup_file_name); }
	bool AutoRecoverRequested() const { return(m_auto_recover_requested); }
	void Recover() const;

private:
	wxString m_backup_file_name;
	int m_save_interval;	// in minutes
	bool m_auto_recover_requested;

}; // End CAutoSafe class definition.


